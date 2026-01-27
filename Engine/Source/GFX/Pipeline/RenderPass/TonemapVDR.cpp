#include "GFX/Pipeline/RenderPass/TonemapVDR.h"
#include "GFX/Resource/Constant.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapVDR
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for TonemapVDR initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	static void SetParams(TonemapParams& params, float midIn, float midOut, float hdrMax) noexcept
	{
		const float hdrShoulderPow = std::pow(hdrMax, params.Shoulder);
		const float hdrShoulderContrastPow = std::pow(hdrShoulderPow, params.Contrast);
		const float hdrShoulderPowMidOut = hdrShoulderPow * midOut;
		const float midInShoulderPow = std::pow(midIn, params.Shoulder);
		const float midInShoulderContrastPow = std::pow(midInShoulderPow, params.Contrast);
		const float paramsBase = (hdrShoulderContrastPow - midInShoulderContrastPow) * midOut;

		params.B = (hdrShoulderPowMidOut - midInShoulderPow) / paramsBase;
		params.C = (hdrShoulderContrastPow * midInShoulderPow - midInShoulderContrastPow * hdrShoulderPowMidOut) / paramsBase;
	}

	PassDesc GetDesc(PixelFormat outputFormat) noexcept
	{
		PassDesc desc{ Base(CorePassType::TonemapVDR) };
		desc.InitializeFormats.emplace_back(outputFormat);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Clean = Clean;
		desc.DebugUI = DebugUI;
		return desc;
	}

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		syncStatus.SyncMain(dev);
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->State.Free(dev);
		delete execData;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Frame
		desc.AddRange({ sizeof(TonemapParams), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Params
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, "TonemapPS_V", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = outputFormat;
		ZE_PSO_SET_NAME(psoDesc, "TonemapVDR");

		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));
		SetParams(passData->Params, passData->MidIn, passData->MidOut, passData->MaxRadiance);

		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("TonemapVDR");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_DRAW_TAG_BEGIN(dev, cl, "TonemapVDR", PixelVal::Cobalt);
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.Scene);
		Resource::Constant<TonemapParams> params(dev, data.Params);
		params.Bind(cl, ctx);
		cl.DrawFullscreen(dev);
		renderData.Buffers.EndRaster(cl);

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(void* data) noexcept
	{
		ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

		if (ImGui::CollapsingHeader("VDR Tonemapper"))
		{
			bool updateParams = false;
			ImGui::Columns(2, "##tonemap_params_vdr", false);
			{
				ImGui::Text("Exposure value");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.01f, FLT_MAX, execData.Params.Exposure,
					ImGui::InputFloat("##exposure_value", &execData.Params.Exposure, 0.1f, 0.0f, "%.2f"));

				ImGui::Text("Contrast");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##vdr_contrast", &execData.Params.Contrast, 0.01f, 0.1f, "%.2f");

				ImGui::Text("Middle IN");
				ImGui::SetNextItemWidth(-1.0f);
				updateParams |= ImGui::InputFloat("##vdr_mid_in", &execData.MidIn, 0.01f, 0.1f, "%.2f");
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Scene max radiance");
				ImGui::SetNextItemWidth(-1.0f);
				updateParams |= GUI::InputClamp(1.0f, FLT_MAX, execData.MaxRadiance,
					ImGui::InputFloat("##vdr_hdr_max", &execData.MaxRadiance, 0.1f, 1.0f, "%.1f"));

				ImGui::Text("Shoulder");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##vdr_shoulder", &execData.Params.Shoulder, 0.001f, 0.01f, "%.3f");

				ImGui::Text("Middle OUT");
				ImGui::SetNextItemWidth(-1.0f);
				updateParams |= ImGui::InputFloat("##vdr_mid_out", &execData.MidOut, 0.01f, 0.1f, "%.2f");
			}
			ImGui::Columns(1);
			ImGui::NewLine();

			if (updateParams)
				SetParams(execData.Params, execData.MidIn, execData.MidOut, execData.MaxRadiance);
		}
	}
}