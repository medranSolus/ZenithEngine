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

	constexpr TonemapParams GetDefaultParams(TonemapperType tonemapper) noexcept
	{
		switch (tonemapper)
		{
		default:
			return
			{
				{
					0.0f,
					1.0f, // Reinhard offset
					2.0f, // Max radiance
					0.0f,
				}, 0.0f
			};
		case TonemapperType::FilmicVDR:
			return
			{
				{
					1.3f, // Contrast
					0.18f, // Mid in
					0.18f, // Mid out
					0.995f // Shoulder
				}, 64.0f, // Max radiance
			};
		case TonemapperType::AgX:
			return
			{
				{
					0.0f,
					1.35f, // Base agx look saturation boost
					1.1f, // Subtle contrast enhancement
					0.18f // Pivot point for contrast (mid-gray)
				}, 0.0f
			};
		}
	}

	constexpr bool UsesReinhardOffset(TonemapperType tonemapper) noexcept
	{
		switch (tonemapper)
		{
		case TonemapperType::Reinhard:
		case TonemapperType::ReinhardExtended:
		case TonemapperType::ReinhardLuma:
		case TonemapperType::ReinhardLumaJodie:
		case TonemapperType::ReinhardLumaPreserveWhite:
			return true;
		default:
			return false;
		}
	}

	constexpr bool UsesMaxWhite(TonemapperType tonemapper) noexcept
	{
		switch (tonemapper)
		{
		case TonemapperType::ReinhardExtended:
		case TonemapperType::ReinhardLumaPreserveWhite:
			return true;
		default:
			return false;
		}
	}

	static TonemapParams GetFilmicVDRParams(float exposure, float midIn, float midOut, float hdrMax, float contrast, float shoulder) noexcept
	{
		const float hdrShoulderPow = std::pow(hdrMax, shoulder);
		const float hdrShoulderContrastPow = std::pow(hdrShoulderPow, contrast);
		const float hdrShoulderPowMidOut = hdrShoulderPow * midOut;
		const float midInShoulderPow = std::pow(midIn, shoulder);
		const float midInShoulderContrastPow = std::pow(midInShoulderPow, contrast);
		const float paramsBase = (hdrShoulderContrastPow - midInShoulderContrastPow) * midOut;

		const float b = (hdrShoulderPowMidOut - midInShoulderPow) / paramsBase;
		const float c = (hdrShoulderContrastPow * midInShoulderPow - midInShoulderContrastPow * hdrShoulderPowMidOut) / paramsBase;

		return { { exposure, shoulder, b, c }, contrast };
	}

	static std::string GetPsoName(TonemapperType tonemapper) noexcept
	{
		std::string base = "TonemapPS";
		if (tonemapper != TonemapperType::None)
			base += '_';
		switch (tonemapper)
		{
		case TonemapperType::Exposure:
			base += 'E';
			break;
		case TonemapperType::Reinhard:
			base += 'R';
			break;
		case TonemapperType::ReinhardExtended:
			base += "RX";
			break;
		case TonemapperType::ReinhardLuma:
			base += "RL";
			break;
		case TonemapperType::ReinhardLumaJodie:
			base += "RJ";
			break;
		case TonemapperType::ReinhardLumaPreserveWhite:
			base += "RW";
			break;
		case TonemapperType::RomBinDaHouse:
			base += 'H';
			break;
		case TonemapperType::FilmicHable:
			base += 'F';
			break;
		case TonemapperType::FilmicVDR:
			base += "FV";
			break;
		case TonemapperType::ACES:
			base += 'A';
			break;
		case TonemapperType::ACESNautilus:
			base += "AN";
			break;
		case TonemapperType::AgX:
			base += 'X';
			break;
		case TonemapperType::KhronosPBRNeutral:
			base += 'K';
			break;
		default:
			break;
		}
		return base;
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
			passData.Params = GetDefaultParams(passData.CurrentTonemapper);

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
		desc.AddRange({ sizeof(TonemapParams), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Exposure | Tonemapper specific parameters
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

		TonemapParams paramsValue = {};
		if (data.CurrentTonemapper == TonemapperType::FilmicVDR)
			paramsValue = GetFilmicVDRParams(data.Exposure, data.Params.Params0.y, data.Params.Params0.z, data.Params.Params1, data.Params.Params0.x, data.Params.Params0.w);
		else
		{
			paramsValue.Params0 = data.Params.Params0;
			paramsValue.Params0.x = data.Exposure;
			paramsValue.Params1 = data.Params.Params1;
		}

		renderData.Buffers.SetSRV(cl, ctx, ids.Scene);
		Resource::Constant<TonemapParams> params(dev, paramsValue);
		params.Bind(cl, ctx);
		cl.DrawFullscreen(dev);
		renderData.Buffers.EndRaster(cl);

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(void* data) noexcept
	{
		ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

		if (ImGui::CollapsingHeader("Tonemapping Collection"))
		{
			ImGui::Columns(2, "##tonemap_params", false);

			ImGui::Text("Exposure value");
			ImGui::SetNextItemWidth(-1.0f);
			GUI::InputClamp(0.01f, FLT_MAX, execData.Exposure,
				ImGui::InputFloat("##exposure_value", &execData.Exposure, 0.1f, 0.0f, "%.2f"));

			switch (execData.CurrentTonemapper)
			{
			case TonemapperType::FilmicVDR:
			{
				ImGui::Text("Contrast");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##vdr_contrast", &execData.Params.Params0.x, 0.01f, 0.1f, "%.2f");

				ImGui::Text("Middle IN");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##vdr_mid_in", &execData.Params.Params0.y, 0.01f, 0.1f, "%.2f");

				ImGui::NextColumn();

				ImGui::Text("Scene max radiance");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(1.0f, FLT_MAX, execData.Params.Params1,
					ImGui::InputFloat("##vdr_hdr_max", &execData.Params.Params1, 0.1f, 1.0f, "%.1f"));

				ImGui::Text("Shoulder");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##vdr_shoulder", &execData.Params.Params0.w, 0.001f, 0.01f, "%.3f");

				ImGui::Text("Middle OUT");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##vdr_mid_out", &execData.Params.Params0.z, 0.01f, 0.1f, "%.2f");
				break;
			}
			case TonemapperType::AgX:
			{
				ImGui::Text("Contrast");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##agx_contrast", &execData.Params.Params0.z, 0.01f, 0.1f, "%.2f");

				ImGui::NextColumn();

				ImGui::Text("Saturation");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##agx_saturation", &execData.Params.Params0.y, 0.01f, 0.1f, "%.2f");

				ImGui::Text("Mid constrast");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::InputFloat("##agx_mid_constrast", &execData.Params.Params0.w, 0.01f, 0.1f, "%.2f");
				break;
			}
			default:
			{
				ImGui::Text("Scene max radiance");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::BeginDisabled(!UsesMaxWhite(execData.CurrentTonemapper));
				GUI::InputClamp(1.0f, FLT_MAX, execData.Params.Params0.z,
					ImGui::InputFloat("##max_radiance", &execData.Params.Params0.z, 0.1f, 1.0f, "%.1f"));
				ImGui::EndDisabled();

				ImGui::NextColumn();

				ImGui::Text("Reinhard offset");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::BeginDisabled(!UsesReinhardOffset(execData.CurrentTonemapper));
				GUI::InputClamp(0.01f, FLT_MAX, execData.Params.Params0.y,
					ImGui::InputFloat("##reihnard_offset", &execData.Params.Params0.y, 0.01f, 0.1f, "%.2f"));
				ImGui::EndDisabled();
				break;
			}
			}

			ImGui::Columns(1);
		}
	}
}