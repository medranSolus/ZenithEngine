#include "GFX/Pipeline/RenderPass/TonemapAgX.h"
#include "GFX/Resource/Constant.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapAgX
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for TonemapAgX initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	PassDesc GetDesc(PixelFormat outputFormat) noexcept
	{
		PassDesc desc{ Base(CorePassType::TonemapAgX) };
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
		desc.AddRange({ sizeof(Float4), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, "TonemapPS_X", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = outputFormat;
		ZE_PSO_SET_NAME(psoDesc, "TonemapAgX");

		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("TonemapAgX");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_DRAW_TAG_BEGIN(dev, cl, "TonemapAgX", PixelVal::Cobalt);
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.Scene);
		Resource::Constant<Float4> params(dev, data.Params);
		params.Bind(cl, ctx);
		cl.DrawFullscreen(dev);
		renderData.Buffers.EndRaster(cl);

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(void* data) noexcept
	{
		ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

		if (ImGui::CollapsingHeader("AgX Tonemapper"))
		{
			ImGui::Columns(2, "##tonemap_params_agx", false);
			{
				ImGui::Text("Exposure value");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.01f, FLT_MAX, execData.Params.x,
					ImGui::InputFloat("##exposure_value", &execData.Params.x, 0.1f, 0.0f, "%.2f"));

				ImGui::Text("Contrast");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##agx_contrast", &execData.Params.z, 0.01f, 0.1f, "%.2f");
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Saturation");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##agx_saturation", &execData.Params.y, 0.01f, 0.1f, "%.2f");

				ImGui::Text("Mid constrast");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##agx_mid_constrast", &execData.Params.w, 0.01f, 0.1f, "%.2f");
			}
			ImGui::Columns(1);
			ImGui::NewLine();
		}
	}
}