#include "GFX/Pipeline/RenderPass/UpscaleNIS.h"
ZE_WARNING_PUSH
#include "NIS_Config.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleNIS
{
	static bool Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) { return Initialize(dev, buildData); }

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ static_cast<PassType>(CorePassType::UpscaleNIS) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		return desc;
	}

	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->StateUpscale.Free(dev);
		execData->Coefficients.Free(dev);
		delete execData;
	}

	bool Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData)
	{
		if (passData.Float16Support != dev.IsShaderFloat16Supported())
		{
			passData.Float16Support = dev.IsShaderFloat16Supported();

			// Choose linear HDR for upscaled data
			passData.BlockHeight = 24;
			std::string shaderName = "NVImageScalingCS_L";
			if (Settings::GpuVendor == VendorGPU::Nvidia)
				shaderName += "NV";
			if (passData.Float16Support)
			{
				shaderName += "H";
				if (Settings::GpuVendor == VendorGPU::Nvidia)
					passData.BlockHeight = 32;
			}
			Resource::Shader upscale(dev, shaderName);
			passData.StateUpscale.Init(dev, upscale, buildData.BindingLib.GetSchema(passData.BindingIndex));
			upscale.Free(dev);

			// Create coefficients textures
			constexpr U32 COEFF_WIDTH = kFilterSize / 4;
			constexpr U32 COEFF_HEIGHT = Utils::SafeCast<U32>(kPhaseCount);
			std::vector<Surface> surfacesScale;
			std::vector<Surface> surfacesUSM;

			if (passData.Float16Support)
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
			coeffDesc.AddTexture(Resource::Texture::Type::Tex2D, std::move(surfacesScale));
			coeffDesc.AddTexture(Resource::Texture::Type::Tex2D, std::move(surfacesUSM));
			passData.Coefficients.Free(dev);
			passData.Coefficients.Init(dev, buildData.Assets.GetDisk(), coeffDesc);
		}

		UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::NIS, static_cast<U32>(passData.Quality));
		if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize)
		{
			Settings::RenderSize = renderSize;
			passData.DisplaySize = Settings::DisplaySize;
			return true;
		}
		return false;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData)
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

		passData->Float16Support = !dev.IsShaderFloat16Supported();
		Update(dev, buildData, *passData);

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& execData, PassData& passData)
	{
		ZE_PERF_GUARD("Upscale NIS");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();
		const UInt2 inputSize = {};// execData.Buffers.GetDimmensions(ids.Color);
		const UInt2 outputSize = {};// execData.Buffers.GetDimmensions(ids.Output);

		NISConfig config = {};
		[[maybe_unused]] const bool correctUpdate = NVScalerUpdateConfig(config, data.Sharpness,
			0, 0, inputSize.X, inputSize.Y, inputSize.X, inputSize.Y,
			0, 0, outputSize.X, outputSize.Y, outputSize.X, outputSize.Y, NISHDRMode::Linear);
		ZE_ASSERT(correctUpdate, "Error updating NIS config data!");

		// TODO: add some begin-end parts for setting up render targets/or something for UAV with compute
		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale NIS", Pixel(0x32, 0xCD, 0x32));
		data.StateUpscale.Bind(cl);

		Binding::Context bindCtx{ execData.Bindings.GetSchema(data.BindingIndex) };
		bindCtx.BindingSchema.SetCompute(cl);

		auto& cbuffer = *execData.DynamicBuffer;
		cbuffer.Bind(cl, bindCtx, cbuffer.Alloc(dev, &config, sizeof(NISConfig)));
		//execData.Buffers.SetUAV(cl, bindCtx, ids.Output);
		//execData.Buffers.SetSRV(cl, bindCtx, ids.Color);
		data.Coefficients.Bind(cl, bindCtx);

		cl.Compute(dev, Math::DivideRoundUp(outputSize.X, 32U), Math::DivideRoundUp(outputSize.Y, data.BlockHeight), 1);
		ZE_DRAW_TAG_END(dev, cl);
	}
}