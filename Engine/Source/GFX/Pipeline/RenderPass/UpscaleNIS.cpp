#include "GFX/Pipeline/RenderPass/UpscaleNIS.h"
#include "GFX/Pipeline/RendererPBR.h"
ZE_WARNING_PUSH
#include "NIS_Config.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleNIS
{
	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->StateUpscale.Free(dev);
		execData->Coefficients.Free(dev);
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Compute, Binding::RangeFlag::CBV }); // NIS constants
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack }); // Output
		desc.AddRange({ 1, 0, 1, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Input
		desc.AddRange({ 2, 1, 2, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Coefficients
		desc.AddSampler(
			{
				Resource::LinearMinification | Resource::LinearMagnification,
				{
					Resource::Texture::AddressMode::Edge,
					Resource::Texture::AddressMode::Edge,
					Resource::Texture::AddressMode::Edge
				},
				0.0f, 1, Resource::CompareMethod::Never,
				Resource::Texture::EdgeColor::TransparentBlack,
				0.0f, FLT_MAX, 0
			});
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		// Choose linear HDR for upscaled data
		passData->BlockHeight = 24;
		std::string shaderName = "NVImageScalingCS_L";
		if (Settings::GpuVendor == VendorGPU::Nvidia)
			shaderName += "NV";
		if (dev.IsShaderFloat16Supported())
		{
			shaderName += "H";
			if (Settings::GpuVendor == VendorGPU::Nvidia)
				passData->BlockHeight = 32;
		}
		Resource::Shader upscale(dev, shaderName);
		passData->StateUpscale.Init(dev, upscale, buildData.BindingLib.GetSchema(passData->BindingIndex));
		upscale.Free(dev);

		// Create coefficients textures
		constexpr U32 COEFF_WIDTH = kFilterSize / 4;
		constexpr U32 COEFF_HEIGHT = Utils::SafeCast<U32>(kPhaseCount);
		std::vector<Surface> surfacesScale;
		std::vector<Surface> surfacesUSM;

		if (dev.IsShaderFloat16Supported())
		{
			surfacesScale.emplace_back(COEFF_WIDTH, COEFF_HEIGHT, PixelFormat::R16G16B16A16_Float, coef_scale_fp16);
			surfacesUSM.emplace_back(COEFF_WIDTH, COEFF_HEIGHT, PixelFormat::R16G16B16A16_Float, coef_usm_fp16);
		}
		else
		{
			surfacesScale.emplace_back(COEFF_WIDTH, COEFF_HEIGHT, PixelFormat::R32G32B32A32_Float, coef_scale);
			surfacesUSM.emplace_back(COEFF_WIDTH, COEFF_HEIGHT, PixelFormat::R32G32B32A32_Float, coef_usm);
		}

		Resource::Texture::PackDesc coeffDesc;
		coeffDesc.Options = Resource::Texture::PackOption::StaticCreation;
		coeffDesc.AddTexture(Resource::Texture::Type::Tex2D, Resource::Texture::Usage::NonPixelShader, std::move(surfacesScale));
		coeffDesc.AddTexture(Resource::Texture::Type::Tex2D, Resource::Texture::Usage::NonPixelShader, std::move(surfacesUSM));
		passData->Coefficients.Init(dev, buildData.Assets.GetDisk(), coeffDesc);

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Upscale NIS");
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);
		const UInt2 outputSize = renderData.Buffers.GetDimmensions(ids.Output);

		cl.Open(dev, data.StateUpscale);
		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale NIS", Pixel(0x32, 0xCD, 0x32));

		RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
		auto& cbuffer = *renderData.DynamicBuffer;

		NISConfig config = {};
		[[maybe_unused]] const bool correctUpdate = NVScalerUpdateConfig(config, renderer.GetSharpness(),
			0, 0, inputSize.X, inputSize.Y, inputSize.X, inputSize.Y,
			0, 0, outputSize.X, outputSize.Y, outputSize.X, outputSize.Y, NISHDRMode::Linear);
		ZE_ASSERT(correctUpdate, "Error updating NIS config data!");
		auto cbufferInfo = cbuffer.Alloc(dev, &config, sizeof(NISConfig));

		Binding::Context bindCtx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		bindCtx.BindingSchema.SetCompute(cl);
		cbuffer.Bind(cl, bindCtx, cbufferInfo);
		renderData.Buffers.SetUAV(cl, bindCtx, ids.Output);
		renderData.Buffers.SetSRV(cl, bindCtx, ids.Color);
		data.Coefficients.Bind(cl, bindCtx);
		cl.Compute(dev, Math::DivideRoundUp(outputSize.X, 32U), Math::DivideRoundUp(outputSize.Y, data.BlockHeight), 1);

		ZE_DRAW_TAG_END(dev, cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}