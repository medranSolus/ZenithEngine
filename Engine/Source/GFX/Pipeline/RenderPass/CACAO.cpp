#include "GFX/Pipeline/RenderPass/CACAO.h"
#include "GFX/FfxBackendInterface.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::CACAO
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) { return Initialize(dev, buildData); }

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::CACAO) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		desc.DebugUI = DebugUI;
		return desc;
	}

	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, bool firstUpdate)
	{
		if (passData.RenderSize != Settings::RenderSize)
		{
			ZE_FFX_ENABLE();
			passData.RenderSize = Settings::RenderSize;

			if (!firstUpdate)
			{
				if (Settings::IsEnabledAsyncAO())
					buildData.SyncStatus.SyncCompute(dev);
				else
					buildData.SyncStatus.SyncMain(dev);
				ZE_FFX_CHECK(ffxCacaoContextDestroy(&passData.Ctx), "Error destroying CACAO context!");
			}
			FfxCacaoContextDescription cacaoDesc = {};
			cacaoDesc.backendInterface = buildData.FfxInterface;
			cacaoDesc.width = passData.RenderSize.X;
			cacaoDesc.height = passData.RenderSize.Y;
			cacaoDesc.useDownsampledSsao = false;
			ZE_FFX_THROW_FAILED(ffxCacaoContextCreate(&passData.Ctx, &cacaoDesc), "Error creating CACAO context!");
			return UpdateStatus::InternalOnly;
		}
		return UpdateStatus::NoUpdate;
	}

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		if (Settings::IsEnabledAsyncAO())
			syncStatus.SyncCompute(dev);
		else
			syncStatus.SyncMain(dev);

		ZE_FFX_ENABLE();
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		ZE_FFX_CHECK(ffxCacaoContextDestroy(&execData->Ctx), "Error destroying CACAO context!");
		delete execData;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData)
	{
		ExecuteData* passData = new ExecuteData;

		passData->Settings.generateNormals = false;
		Update(dev, buildData, *passData, true);

		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("CACAO");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_DRAW_TAG_BEGIN(dev, cl, "CACAO", Pixel(0x89, 0xCF, 0xF0));

		data.Settings.temporalSupersamplingAngleOffset = Math::PI * Utils::SafeCast<float>(Settings::GetFrameIndex() % 3) / 3.0f;
		data.Settings.temporalSupersamplingRadiusOffset = 1.0f + (Utils::SafeCast<float>(Settings::GetFrameIndex() % 3) - 1.0f) / 30.0f;
		ZE_FFX_CHECK(ffxCacaoUpdateSettings(&data.Ctx, &data.Settings, false), "Error updating CACAO settings!");

		FfxCacaoDispatchDescription desc = {};
		desc.commandList = FFX::GetCommandList(cl);
		desc.depthBuffer = FFX::GetResource(renderData.Buffers, ids.Depth, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.normalBuffer = FFX::GetResource(renderData.Buffers, ids.Normal, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.outputBuffer = FFX::GetResource(renderData.Buffers, ids.AO, FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		// Matrix data is not modified inside callbacks, missing const specifier in header
		desc.proj = reinterpret_cast<FfxFloat32x4x4*>(&renderData.GraphData.Projection);
		desc.normalsToView = reinterpret_cast<FfxFloat32x4x4*>(&renderData.DynamicData.ViewTps);
		// Custom way of loading normals is chosen so no need to perform any unpacking from SDK (custom callbacks provided)
		desc.normalUnpackMul = 1.0f;
		desc.normalUnpackAdd = 0.0f;
		ZE_FFX_THROW_FAILED(ffxCacaoContextDispatch(&data.Ctx, &desc), "Error performing CACAO!");

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(void* data) noexcept
	{
		if (ImGui::CollapsingHeader("CACAO"))
		{
			ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

			ImGui::Text("Version " ZE_STRINGIFY_VERSION(ZE_DEPAREN(FFX_CACAO_VERSION_MAJOR), ZE_DEPAREN(FFX_CACAO_VERSION_MINOR), ZE_DEPAREN(FFX_CACAO_VERSION_PATCH)));

			constexpr std::array<const char*, 5> LEVELS = { "Lowest", "Low", "Medium", "High", "Highest" };
			if (ImGui::BeginCombo("Quality level", LEVELS.at(execData.Settings.qualityLevel)))
			{
				for (FfxCacaoQuality i = FFX_CACAO_QUALITY_LOWEST; const char* level : LEVELS)
				{
					const bool selected = i == execData.Settings.qualityLevel;
					if (ImGui::Selectable(level, selected))
						execData.Settings.qualityLevel = i;
					if (selected)
						ImGui::SetItemDefaultFocus();
					i = static_cast<FfxCacaoQuality>(i + 1);
				}
				ImGui::EndCombo();
			}

			ImGui::Columns(2, "##cacao_options", false);
			{
				ImGui::Text("Blur radius");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.1f, FLT_MAX, execData.Settings.radius,
					ImGui::InputFloat("##cacao_blur_radius", &execData.Settings.radius, 0.1f, 1.0f, "%.1f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Size of the occlusion sphere");
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Blur pass count");
				ImGui::SetNextItemWidth(-1.0f);
				ImGui::SliderInt("##cacao_blur_count", reinterpret_cast<int*>(&execData.Settings.blurPassCount), 0, 8);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Number of edge-sensitive smart blur passes to apply");
			}
			ImGui::Columns(1);

			if (execData.Settings.qualityLevel != FFX_CACAO_QUALITY_HIGHEST)
				ImGui::BeginDisabled(true);
			ImGui::Text("Adaptative quality limit");
			ImGui::InputFloat("##cacao_adapt_limit", &execData.Settings.adaptiveQualityLimit, 0.01f, 0.1f, "%.2f");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Only for highest quality level");
			if (execData.Settings.qualityLevel != FFX_CACAO_QUALITY_HIGHEST)
				ImGui::EndDisabled();

			if (ImGui::CollapsingHeader("Advanced"))
			{
				ImGui::Columns(2, "##cacao_advanced", false);
				{
					ImGui::Text("Sharpness");
					GUI::InputClamp(0.0f, 1.0f, execData.Settings.sharpness,
						ImGui::InputFloat("##cacao_sharpness", &execData.Settings.sharpness, 0.01f, 0.1f, "%.2f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("How much to bleed over edges; 1: not at all, 0.5: half-half; 0.0: completely ignore edges");
					ImGui::Text("Fade out start");
					GUI::InputClamp(0.0f, execData.Settings.fadeOutTo, execData.Settings.fadeOutFrom,
						ImGui::InputFloat("##cacao_fade_from", &execData.Settings.fadeOutFrom, 1.0f, 5.0f, "%.1f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Distance to start fading out the effect");
				}
				ImGui::NextColumn();
				{
					ImGui::Text("Horizontal angle treshold");
					GUI::InputClamp(0.0f, 0.2f, execData.Settings.horizonAngleThreshold,
						ImGui::InputFloat("##cacao_horizon_angle", &execData.Settings.horizonAngleThreshold, 0.01f, 0.0f, "%.2f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Limits self-shadowing (makes the sampling area less of a hemisphere, more of a spherical cone, to avoid self-shadowing and various artifacts due to low tessellation and depth buffer imprecision, etc.)");
					ImGui::Text("Fade out end");
					GUI::InputClamp(execData.Settings.fadeOutFrom, FLT_MAX, execData.Settings.fadeOutTo,
						ImGui::InputFloat("##cacao_fade_to", &execData.Settings.fadeOutTo, 1.0f, 5.0f, "%.1f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Distance at which the effect is faded out");
				}
				ImGui::Columns(1);

				ImGui::Text("Shadow controls");
				ImGui::Columns(2, "##cacao_shadows", false);
				{
					ImGui::Text("Shadow multipler");
					GUI::InputClamp(0.0f, 5.0f, execData.Settings.shadowMultiplier,
						ImGui::InputFloat("##cacao_shadow_mult", &execData.Settings.shadowMultiplier, 0.1f, 1.0f, "%.1f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Effect strength linear multiplier");
					ImGui::Text("Shadow clamp");
					GUI::InputClamp(0.0f, 1.0f, execData.Settings.shadowClamp,
						ImGui::InputFloat("##cacao_shadow_clamp", &execData.Settings.shadowClamp, 0.01f, 0.1f, "%.2f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Effect max limit (applied after multiplier but before blur)");
				}
				ImGui::NextColumn();
				{
					ImGui::Text("Shadow power");
					GUI::InputClamp(0.5f, 5.0f, execData.Settings.shadowPower,
						ImGui::InputFloat("##cacao_shadow_pow", &execData.Settings.shadowPower, 0.01f, 0.1f, "%.2f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Effect strength modifier");
					ImGui::Text("Shadow detail");
					GUI::InputClamp(0.5f, 5.0f, execData.Settings.detailShadowStrength,
						ImGui::InputFloat("##cacao_shadow_det", &execData.Settings.detailShadowStrength, 0.1f, 1.0f, "%.1f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Used for high-res detail AO using neighboring depth pixels: adds a lot of detail but also reduces temporal stability (adds aliasing)");
				}
				ImGui::Columns(1);

				ImGui::Text("Bilateral sigma");
				ImGui::Columns(2, "##cacao_bilateral", false);
				{
					ImGui::Text("Blur term");
					GUI::InputClamp(0.0f, FLT_MAX, execData.Settings.bilateralSigmaSquared,
						ImGui::InputFloat("##cacao_bil_sigma", &execData.Settings.bilateralSigmaSquared, 0.1f, 1.0f, "%.1f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Sigma squared value for use in bilateral upsampler giving Gaussian blur term");
				}
				ImGui::NextColumn();
				{
					ImGui::Text("Similarity weight");
					GUI::InputClamp(0.0f, FLT_MAX, execData.Settings.bilateralSimilarityDistanceSigma,
						ImGui::InputFloat("##cacao_bil_similarity", &execData.Settings.bilateralSimilarityDistanceSigma, 0.01f, 0.1f, "%.2f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Sigma squared value for use in bilateral upsampler giving similarity weighting for neighbouring pixels");
				}
				ImGui::Columns(1);
				ImGui::NewLine();
			}
			ImGui::NewLine();
		}
	}
}