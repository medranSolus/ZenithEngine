#include "GFX/Pipeline/RenderPass/UpscaleNIS.h"
#include "GUI/DearImGui.h"
ZE_WARNING_PUSH
#include "NIS_Config.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleNIS
{
	static Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, PassExecuteData* passData, const std::vector<PixelFormat>& formats) noexcept
	{
		return Update(dev, buildData, *static_cast<ExecuteData*>(passData));
	}

	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) noexcept
	{
		return Initialize(dev, buildData);
	}

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::UpscaleNIS) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.DebugUI = DebugUI;
		return desc;
	}

	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept
	{
		UpdateOperation status = UpdateOperation::NoUpdate;
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
			Resource::Shader shader;
			ZE_EXPECT_RET_FAILED(shader, Resource::Shader::Create(dev, shaderName));
			ZE_EXPECT_RET_FAILED(passData.StateUpscale, Resource::PipelineStateCompute::Create(dev, shader, buildData.BindingLib.GetSchema(passData.BindingIndex)));

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

			Resource::Texture::PackDesc coeffDesc = {};
			ZE_TEXTURE_SET_NAME(coeffDesc, "NIS Coefficients");
			coeffDesc.Options = Resource::Texture::PackOption::StaticCreation;
			coeffDesc.AddTexture(Resource::Texture::Type::Tex2D, std::move(surfacesScale));
			coeffDesc.AddTexture(Resource::Texture::Type::Tex2D, std::move(surfacesUSM));
			ZE_EXPECT_RET_FAILED(passData.Coefficients, Resource::Texture::Pack::Create(dev, buildData.Assets.GetDisk(), coeffDesc));
			status = UpdateOperation::GpuUploadRequired;
		}

		UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::NIS, static_cast<U32>(passData.Quality));
		if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize)
		{
			Settings::RenderSize = renderSize;
			passData.DisplaySize = Settings::DisplaySize;
			status = status == UpdateOperation::GpuUploadRequired ? UpdateOperation::FrameBufferImpactGpuUpload : UpdateOperation::FrameBufferImpact;
		}
		return status;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();

		Binding::SchemaDesc desc = {};
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
		ZE_EXPECT_RET_FAILED(passData->BindingIndex, buildData.BindingLib.AddDataBinding(dev, desc));

		passData->Float16Support = !dev.IsShaderFloat16Supported();
		auto operation = Update(dev, buildData, *passData);
		if (!operation)
			return std::unexpected(operation.error());
		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		ZE_PERF_GUARD("Upscale NIS");

		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);
		const UInt2 outputSize = renderData.Buffers.GetDimmensions(ids.Output);

		NISConfig config = {};
		[[maybe_unused]] const bool correctUpdate = NVScalerUpdateConfig(config, data.SharpeningEnabled ? data.Sharpness : 0.0f,
			0, 0, inputSize.X, inputSize.Y, inputSize.X, inputSize.Y,
			0, 0, outputSize.X, outputSize.Y, outputSize.X, outputSize.Y, NISHDRMode::Linear);
		ZE_ASSERT(correctUpdate, "Error updating NIS config data!");

		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale NIS", Pixel(0x32, 0xCD, 0x32));

		Binding::Context bindCtx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		bindCtx.BindingSchema.SetCompute(cl);
		data.StateUpscale.Bind(cl);

		auto& cbuffer = *renderData.DynamicBuffer;
		ZE_CODE_RET_FAILED_EXPECT(cbuffer.AllocBind(dev, cl, bindCtx, &config, sizeof(NISConfig)));
		renderData.Buffers.SetUAV(cl, bindCtx, ids.Output);
		renderData.Buffers.SetSRV(cl, bindCtx, ids.Color);
		data.Coefficients.Bind(cl, bindCtx);

		cl.Compute(dev, Math::DivideRoundUp(outputSize.X, 32U), Math::DivideRoundUp(outputSize.Y, data.BlockHeight), 1);
		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		if (ImGui::CollapsingHeader("NIS"))
		{
			ExecuteData& execData = *static_cast<ExecuteData*>(data);

			ImGui::Text("Version 1.0.3");

			constexpr std::array<const char*, 5> LEVELS = { "Performance", "Balanced", "Quality", "Ultra Quality", "Mega Quality" };
			if (ImGui::BeginCombo("Quality level", LEVELS.at(4U - static_cast<U8>(execData.Quality))))
			{
				for (NISQualityMode i = NISQualityMode::Performance; const char* level : LEVELS)
				{
					const bool selected = i == execData.Quality;
					if (ImGui::Selectable(level, selected))
						execData.Quality = i;
					if (selected)
						ImGui::SetItemDefaultFocus();
					i = static_cast<NISQualityMode>(static_cast<U8>(i) - 1U);
				}
				ImGui::EndCombo();
			}

			ImGui::Columns(2, "##sharpness_settings", false);
			{
				ImGui::Text("Sharpness");
			}
			ImGui::NextColumn();
			{
				ImGui::Checkbox("##enable_sharpness", &execData.SharpeningEnabled);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable an additional sharpening pass");
			}
			ImGui::Columns(1);

			if (!execData.SharpeningEnabled)
				ImGui::BeginDisabled(true);
			GUI::InputClamp(0.0f, 1.0f, execData.Sharpness,
				ImGui::InputFloat("##nis_sharpness", &execData.Sharpness, 0.01f, 0.1f, "%.2f"));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("The sharpness value between 0 and 1, where 0 is no additional sharpness and 1 is maximum additional sharpness");
			if (!execData.SharpeningEnabled)
				ImGui::EndDisabled();
			ImGui::NewLine();
		}
	}
}