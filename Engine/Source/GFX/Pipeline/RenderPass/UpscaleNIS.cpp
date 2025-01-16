#include "GFX/Pipeline/RenderPass/UpscaleNIS.h"
#include "GUI/DearImGui.h"
ZE_WARNING_PUSH
#include "NIS_Config.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::UpscaleNIS
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) { return Initialize(dev, buildData); }

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::UpscaleNIS) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		desc.DebugUI = DebugUI;
		return desc;
	}

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		Settings::RenderSize = Settings::DisplaySize;
		syncStatus.SyncMain(dev);
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->StateUpscale.Free(dev);
		execData->Coefficients.Free(dev);
		delete execData;
	}

	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData)
	{
		UpdateStatus status = UpdateStatus::NoUpdate;
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
			buildData.SyncStatus.SyncMain(dev);
			passData.StateUpscale.Free(dev);
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
			ZE_TEXTURE_SET_NAME(coeffDesc, "NIS Coefficients");
			coeffDesc.Options = Resource::Texture::PackOption::StaticCreation;
			coeffDesc.AddTexture(Resource::Texture::Type::Tex2D, std::move(surfacesScale));
			coeffDesc.AddTexture(Resource::Texture::Type::Tex2D, std::move(surfacesUSM));
			passData.Coefficients.Free(dev);
			passData.Coefficients.Init(dev, buildData.Assets.GetDisk(), coeffDesc);
			status = UpdateStatus::GpuUploadRequired;
		}

		UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::NIS, static_cast<U32>(passData.Quality));
		if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize)
		{
			Settings::RenderSize = renderSize;
			passData.DisplaySize = Settings::DisplaySize;
			status = status == UpdateStatus::GpuUploadRequired ? UpdateStatus::FrameBufferImpactGpuUpload : UpdateStatus::FrameBufferImpact;
		}
		return status;
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

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Upscale NIS");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();
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
		cbuffer.Bind(cl, bindCtx, cbuffer.Alloc(dev, &config, sizeof(NISConfig)));
		renderData.Buffers.SetUAV(cl, bindCtx, ids.Output);
		renderData.Buffers.SetSRV(cl, bindCtx, ids.Color);
		data.Coefficients.Bind(cl, bindCtx);

		cl.Compute(dev, Math::DivideRoundUp(outputSize.X, 32U), Math::DivideRoundUp(outputSize.Y, data.BlockHeight), 1);
		ZE_DRAW_TAG_END(dev, cl);
	}

	void DebugUI(void* data) noexcept
	{
		if (ImGui::CollapsingHeader("NIS"))
		{
			ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

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