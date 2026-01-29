#include "GFX/Pipeline/RenderPass/TonemapCollection.h"
#include "GFX/Resource/Constant.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapCollection
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for TonemapCollection initialization formats!");
		return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData), formats.front());
	}

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for TonemapCollection initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	static std::string GetPsoName(TonemapperType tonemapper) noexcept
	{
		std::string base = "TonemapPS";
		if (tonemapper != TonemapperType::None)
			base += '_';
		switch (tonemapper)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case TonemapperType::Exposure:
			base += 'E';
			break;
		case TonemapperType::RomBinDaHouse:
			base += 'H';
			break;
		case TonemapperType::FilmicHable:
			base += 'F';
			break;
		case TonemapperType::ACES:
			base += 'A';
			break;
		case TonemapperType::ACESNautilus:
			base += "AN";
			break;
		case TonemapperType::KhronosPBRNeutral:
			base += 'K';
			break;
		case TonemapperType::None:
			break;
		}
		return base;
	}

	bool Evaluate() noexcept
	{
		switch (Settings::Tonemapper)
		{
		case TonemapperType::None:
		case TonemapperType::Exposure:
		case TonemapperType::RomBinDaHouse:
		case TonemapperType::FilmicHable:
		case TonemapperType::ACES:
		case TonemapperType::ACESNautilus:
		case TonemapperType::KhronosPBRNeutral:
			return true;
		default:
			return false;
		}
	}

	PassDesc GetDesc(PixelFormat outputFormat) noexcept
	{
		PassDesc desc{ Base(CorePassType::TonemapCollection) };
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
		desc.AddRange({ sizeof(float), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Exposure
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		passData->CurrentTonemapper = TonemapperType::LPM;
		Update(dev, buildData, *passData, outputFormat, true);
		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("TonemapCollection");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_DRAW_TAG_BEGIN(dev, cl, "TonemapCollection", PixelVal::Cobalt);
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.Scene);
		Resource::Constant<float> params(dev, data.Exposure);
		params.Bind(cl, ctx);
		cl.DrawFullscreen(dev);
		renderData.Buffers.EndRaster(cl);

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(void* data) noexcept
	{
		if (ImGui::CollapsingHeader("Tonemapping Collection"))
		{
			ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

			ImGui::Columns(2, "##tonemap_params", false);

			ImGui::Text("Exposure value");
			ImGui::SetNextItemWidth(-1.0f);
			GUI::InputClamp(0.01f, FLT_MAX, execData.Exposure,
				ImGui::InputFloat("##exposure_value", &execData.Exposure, 0.1f, 0.0f, "%.2f"));
			ImGui::Columns(1);
			ImGui::NewLine();
		}
	}
}