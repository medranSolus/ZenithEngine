#include "GFX/Pipeline/RenderPass/TonemapGT7.h"
#include "GFX/Resource/Constant.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapGT7
{
	static Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, PassExecuteData* passData, const std::vector<PixelFormat>& formats) noexcept
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for TonemapGT7 initialization formats!");
		return Update(dev, buildData, *static_cast<ExecuteData*>(passData), formats.front());
	}

	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, PassInitData* initData) noexcept
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for TonemapGT7 initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	static void SetParams(TonemapParams& params, float alpha, float linearSection, float targetNits, bool useJzazbz) noexcept
	{
		// Regarding SDR output:
		// First, in GT (Gran Turismo), it is assumed that a maximum value of 1 in SDR output
		// corresponds to GT7_SDR_PAPER_WHITE (typically 250 nits).
		// Therefore, tone mapping for SDR output is performed based on GT7_SDR_PAPER_WHITE.
		// However, in the sRGB standard, 1 corresponds to 100 nits,
		// so we need to "undo" the tone-mapped values accordingly.
		// To match the sRGB range, the tone-mapped values are scaled using CorrectionSDR.
		//
		// * These adjustments ensure that the visual appearance (in terms of brightness)
		//   stays generally consistent across both HDR and SDR outputs for the same rendered content.
		if (targetNits <= GT7_SDR_PAPER_WHITE)
		{
			params.CorrectionSDR = Math::Light::REFERENCE_LUMINANCE / GT7_SDR_PAPER_WHITE;
			targetNits = GT7_SDR_PAPER_WHITE;
		}
		else
			params.CorrectionSDR = 1.0f;

		params.LuminanceTarget = targetNits / Math::Light::REFERENCE_LUMINANCE;
		params.ShoulderTreshold = linearSection * params.LuminanceTarget;

		if (useJzazbz)
		{
			float l = params.LuminanceTarget * 0.530004f + params.LuminanceTarget * 0.355704f + params.LuminanceTarget * 0.086090f;
			float m = params.LuminanceTarget * 0.289388f + params.LuminanceTarget * 0.525395f + params.LuminanceTarget * 0.157481f;

			float lPQ = Math::Light::ApplyPQ(l, 1.7f);
			float mPQ = Math::Light::ApplyPQ(m, 1.7f);

			float iz = 0.5f * (lPQ + mPQ);

			params.LuminanceTargetUCS = (0.44f * iz) / (1.0f - 0.56f * iz) - 1.6295499532821566e-11f;
		}
		else
		{
			float l = params.LuminanceTarget * 0.412109375f + params.LuminanceTarget * 0.52392578125f + params.LuminanceTarget * 0.06396484375f;
			float m = params.LuminanceTarget * 0.166748046875f + params.LuminanceTarget * 0.720458984375f + params.LuminanceTarget * 0.11279296875f;

			float lPQ = Math::Light::ApplyPQ(l, 1.0f);
			float mPQ = Math::Light::ApplyPQ(m, 1.0f);

			params.LuminanceTargetUCS = 0.5f * (lPQ + mPQ);
		}

		// Pre-compute constants for the shoulder region.
		const float k = (linearSection - 1.0f) / (alpha - 1.0f);
		params.ParamA = params.LuminanceTarget * linearSection + params.LuminanceTarget * k;
		params.ParamB = -params.LuminanceTarget * k * std::expf(linearSection / k);
		params.ParamC = -1.0f / (k * params.LuminanceTarget);
	}

	PassDesc GetDesc(PixelFormat outputFormat) noexcept
	{
		PassDesc desc{ Base(CorePassType::TonemapGT7) };
		desc.InitializeFormats.emplace_back(outputFormat);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.DebugUI = DebugUI;
		return desc;
	}

	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat) noexcept
	{
		UpdateOperation status = UpdateOperation::NoUpdate;
		if (passData.UpdatePso)
		{
			passData.UpdatePso = false;

			Resource::PipelineStateDesc psoDesc = {};
			ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache));
			ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.PS, passData.UseJzazbz ? "TonemapPS_GT7J" : "TonemapPS_GT7", buildData.ShaderCache));
			psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
			psoDesc.Culling = Resource::CullMode::Back;
			psoDesc.RenderTargetsCount = 1;
			psoDesc.FormatsRT[0] = outputFormat;
			ZE_PSO_SET_NAME(psoDesc, passData.UseJzazbz ? "TonemapGT7_Jzazbz" : "TonemapGT7");

			ZE_EXPECT_RET_FAILED(passData.State, Resource::PipelineStateGfx::Create(dev, psoDesc, buildData.BindingLib.GetSchema(passData.BindingIndex)));

			status = UpdateOperation::InternalOnly;
		}
		if (passData.UpdateData)
		{
			passData.UpdateData = false;
			ZE_CODE_RET_FAILED_EXPECT(passData.ParamsBuffer.Update(dev, buildData.Assets.GetDisk(), { INVALID_EID, &passData.Params, nullptr, sizeof(TonemapParams) }));

			status = UpdateOperation::GpuUploadRequired;
		}
		return status;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();

		Binding::SchemaDesc desc = {};
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Frame
		desc.AddRange({ 1, 0, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Params
		desc.AddRange({ sizeof(float), 1, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Exposure
		desc.AppendSamplers(buildData.Samplers);
		ZE_EXPECT_RET_FAILED(passData->BindingIndex, buildData.BindingLib.AddDataBinding(dev, desc));

		passData->UpdatePso = true;
		auto operation = Update(dev, buildData, *passData, outputFormat);
		if (!operation)
			return std::unexpected(operation.error());

		SetParams(passData->Params, passData->Alpha, passData->LinearSection, GT7_SDR_PAPER_WHITE, passData->UseJzazbz);
		ZE_EXPECT_RET_FAILED(passData->ParamsBuffer, Resource::CBuffer::Create(dev, buildData.Assets.GetDisk(), { INVALID_EID, &passData->Params, nullptr, sizeof(TonemapParams) }));

		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		ZE_PERF_GUARD("TonemapGT7");
		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

		ZE_DRAW_TAG_BEGIN(dev, cl, "TonemapGT7", PixelVal::Cobalt);
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.Scene);
		data.ParamsBuffer.Bind(cl, ctx);
		Resource::Constant<float> exposure;
		ZE_EXPECT_RET_FAILED(exposure, Resource::Constant<float>::Create(dev, data.Exposure));
		exposure.Bind(cl, ctx);
		cl.DrawFullscreen(dev);
		renderData.Buffers.EndRaster(cl);

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		if (ImGui::CollapsingHeader("GT7 Tonemapper"))
		{
			ExecuteData& execData = *static_cast<ExecuteData*>(data);

			bool updateParams = false;
			ImGui::Columns(2, "##tonemap_params_gt7", false);
			{
				ImGui::Text("Exposure value");
				ImGui::SetNextItemWidth(-1.0f);
				execData.UpdateData |= GUI::InputClamp(0.01f, FLT_MAX, execData.Exposure,
					ImGui::InputFloat("##exposure_value", &execData.Exposure, 0.1f, 0.0f, "%.2f"));

				ImGui::Text("Fade-in start");
				ImGui::SetNextItemWidth(-1.0f);
				execData.UpdateData |= ImGui::InputFloat("##fade_start", &execData.Params.FadeStart, 0.01f, 0.1f, "%.2f");

				ImGui::Text("Toe strength");
				ImGui::SetNextItemWidth(-1.0f);
				execData.UpdateData |= ImGui::InputFloat("##toe_strength", &execData.Params.ToeStrength, 0.001f, 0.01f, "%.3f");

				ImGui::Text("Linear section");
				ImGui::SetNextItemWidth(-1.0f);
				updateParams |= ImGui::InputFloat("##linear_section", &execData.LinearSection, 0.001f, 0.01f, "%.3f");

				ImGui::NewLine();
				execData.UpdatePso |= ImGui::Checkbox("Use Jzazbz UCS", &execData.UseJzazbz);
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Gray point");
				ImGui::SetNextItemWidth(-1.0f);
				execData.UpdateData |= ImGui::InputFloat("##mid_point", &execData.Params.MidPoint, 0.001f, 0.01f, "%.3f");

				ImGui::Text("Fade-in end");
				ImGui::SetNextItemWidth(-1.0f);
				execData.UpdateData |= ImGui::InputFloat("##fade_end", &execData.Params.FadeEnd, 0.01f, 0.1f, "%.2f");

				ImGui::Text("Blend ratio");
				ImGui::SetNextItemWidth(-1.0f);
				execData.UpdateData |= ImGui::InputFloat("##blend_ratio", &execData.Params.BlendRatio, 0.01f, 0.1f, "%.2f");

				ImGui::Text("Alpha");
				ImGui::SetNextItemWidth(-1.0f);
				updateParams |= ImGui::InputFloat("##alpha", &execData.Alpha, 0.01f, 0.1f, "%.2f");
			}
			ImGui::Columns(1);
			ImGui::NewLine();

			updateParams |= execData.UpdatePso;
			execData.UpdateData |= updateParams;
			if (updateParams)
				SetParams(execData.Params, execData.Alpha, execData.LinearSection, GT7_SDR_PAPER_WHITE, execData.UseJzazbz);
		}
	}
}