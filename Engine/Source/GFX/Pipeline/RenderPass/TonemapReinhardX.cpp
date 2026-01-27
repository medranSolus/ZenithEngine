#include "GFX/Pipeline/RenderPass/TonemapReinhardX.h"
#include "GFX/Resource/Constant.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapReinhardX
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for TonemapReinhardX initialization formats!");
		return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData), formats.front());
	}

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for TonemapReinhardX initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	static std::string GetPsoName(TonemapperType tonemapper) noexcept
	{
		std::string base = "TonemapPS_";
		switch (tonemapper)
		{
		case TonemapperType::ReinhardExtended:
			base += "RX";
			break;
		case TonemapperType::ReinhardLumaPreserveWhite:
			base += "RW";
			break;
		default:
			break;
		}
		return base;
	}

	PassDesc GetDesc(PixelFormat outputFormat) noexcept
	{
		PassDesc desc{ Base(CorePassType::TonemapReinhardX) };
		desc.InitializeFormats.emplace_back(outputFormat);
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
		syncStatus.SyncMain(dev);
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->State.Free(dev);
		delete execData;
	}

	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat, bool firstCall)
	{
		if (passData.CurrentTonemapper != Settings::Tonemapper)
		{
			passData.CurrentTonemapper = Settings::Tonemapper;

			if (!firstCall)
			{
				buildData.SyncStatus.SyncMain(dev);
				passData.State.Free(dev);
			}

			Resource::PipelineStateDesc psoDesc;
			psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
			psoDesc.SetShader(dev, psoDesc.PS, GetPsoName(passData.CurrentTonemapper), buildData.ShaderCache);
			psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
			psoDesc.Culling = Resource::CullMode::Back;
			psoDesc.RenderTargetsCount = 1;
			psoDesc.FormatsRT[0] = outputFormat;
			ZE_PSO_SET_NAME(psoDesc, GetPsoName(passData.CurrentTonemapper));

			passData.State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData.BindingIndex));

			return UpdateStatus::InternalOnly;
		}
		return UpdateStatus::NoUpdate;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Frame
		desc.AddRange({ sizeof(Float3), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		passData->CurrentTonemapper = TonemapperType::None;
		Update(dev, buildData, *passData, outputFormat, true);
		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("TonemapReinhardX");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_DRAW_TAG_BEGIN(dev, cl, "TonemapReinhardX", PixelVal::Cobalt);
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.Scene);
		Resource::Constant<Float3> params(dev, data.Params);
		params.Bind(cl, ctx);
		cl.DrawFullscreen(dev);
		renderData.Buffers.EndRaster(cl);

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(void* data) noexcept
	{
		ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

		if (ImGui::CollapsingHeader("Reinhard X Tonemappers"))
		{
			ImGui::Columns(2, "##tonemap_params_reinhardx", false);
			{
				ImGui::Text("Exposure value");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.01f, FLT_MAX, execData.Params.x,
					ImGui::InputFloat("##exposure_value", &execData.Params.x, 0.1f, 0.0f, "%.2f"));

				ImGui::Text("Scene max radiance");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(1.0f, FLT_MAX, execData.Params.z,
					ImGui::InputFloat("##max_radiance", &execData.Params.z, 0.1f, 1.0f, "%.1f"));
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Reinhard offset");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.01f, FLT_MAX, execData.Params.y,
					ImGui::InputFloat("##reihnard_offset", &execData.Params.y, 0.01f, 0.1f, "%.2f"));
			}
			ImGui::Columns(1);
			ImGui::NewLine();
		}
	}
}