#include "GFX/Pipeline/RenderPass/SSSR.h"
#include "GFX/FfxBackendInterface.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::SSSR
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) { return Initialize(dev, buildData); }

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::SSSR) };
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
		syncStatus.SyncCompute(dev);

		ZE_FFX_ENABLE();
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		ZE_FFX_CHECK(ffxSssrContextDestroy(&execData->Ctx), "Error destroying SSSR context!");
		delete execData;
	}

	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, bool firstUpdate)
	{
		if (passData.RenderSize != Settings::RenderSize)
		{
			ZE_FFX_ENABLE();
			passData.RenderSize = Settings::RenderSize;

			if (!firstUpdate)
			{
				buildData.SyncStatus.SyncMain(dev);
				buildData.SyncStatus.SyncCompute(dev);
				ZE_FFX_CHECK(ffxSssrContextDestroy(&passData.Ctx), "Error destroying SSSR context!");
			}
			FfxSssrContextDescription sssrDesc = {};
			sssrDesc.flags = FFX_SSSR_ENABLE_DEPTH_INVERTED;
			sssrDesc.renderSize.width = passData.RenderSize.X;
			sssrDesc.renderSize.height = passData.RenderSize.Y;
			sssrDesc.normalsHistoryBufferFormat = FFX::GetSurfaceFormat(PixelFormat::R16G16_Float);
			sssrDesc.backendInterface = buildData.FfxInterface;
			ZE_FFX_THROW_FAILED(ffxSssrContextCreate(&passData.Ctx, &sssrDesc), "Error creating SSSR context!");
			return UpdateStatus::InternalOnly;
		}
		return UpdateStatus::NoUpdate;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData)
	{
		ExecuteData* passData = new ExecuteData;
		Update(dev, buildData, *passData, true);
		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("SSSR");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);

		ZE_DRAW_TAG_BEGIN(dev, cl, "SSSR", Pixel(0x80, 0x00, 0x00));

		FfxSssrDispatchDescription desc = {};
		desc.commandList = FFX::GetCommandList(cl);

		desc.color = FFX::GetResource(renderData.Buffers, ids.Color, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.depth = FFX::GetResource(renderData.Buffers, ids.Depth, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.motionVectors = FFX::GetResource(renderData.Buffers, ids.MotionVectors, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.normal = FFX::GetResource(renderData.Buffers, ids.NormalMap, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.materialParameters = FFX::GetResource(renderData.Buffers, ids.MaterialData, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.environmentMap = FFX::GetResource(renderData.Buffers, ids.EnvironmentMap, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.brdfTexture = FFX::GetResource(renderData.Buffers, ids.BrdfLut, FFX_RESOURCE_STATE_COMPUTE_READ);
		desc.output = FFX::GetResource(renderData.Buffers, ids.SSSR, FFX_RESOURCE_STATE_UNORDERED_ACCESS);

		Matrix proj = Math::XMLoadFloat4x4(&renderData.GraphData.Projection);
		Matrix view = Math::XMMatrixTranspose(Math::XMLoadFloat4x4(&renderData.DynamicData.ViewTps));

		Math::XMStoreFloat4x4(reinterpret_cast<Float4x4*>(desc.invViewProjection), Math::XMMatrixTranspose(Math::XMLoadFloat4x4(&renderData.DynamicData.ViewProjectionInverseTps)));
		Math::XMStoreFloat4x4(reinterpret_cast<Float4x4*>(desc.projection), proj);
		Math::XMStoreFloat4x4(reinterpret_cast<Float4x4*>(desc.invProjection), Math::XMMatrixInverse(nullptr, proj));
		Math::XMStoreFloat4x4(reinterpret_cast<Float4x4*>(desc.view), view);
		Math::XMStoreFloat4x4(reinterpret_cast<Float4x4*>(desc.invView), Math::XMMatrixInverse(nullptr, view));
		// FFX SDK used post multiplication in the shaders so combined matrices needs to be recomputed here
		Math::XMStoreFloat4x4(reinterpret_cast<Float4x4*>(desc.prevViewProjection),
			Math::XMMatrixTranspose(Math::XMLoadFloat4x4(&renderData.GraphData.PrevProjection)
				* Math::XMMatrixTranspose(Math::XMLoadFloat4x4(&renderData.GraphData.PrevViewTps))));

		desc.renderSize = { inputSize.X, inputSize.Y };
		desc.motionVectorScale.x = -1.0f;
		desc.motionVectorScale.y = -1.0f;
		desc.iblFactor = data.IblFactor;
		// Custom way of loading normals is chosen so no need to perform any unpacking from SDK (custom callbacks provided)
		desc.normalUnPackMul = 1.0f;
		desc.normalUnPackAdd = 0.0f;
		desc.roughnessChannel = 0; // Not used, specified directly in shader
		desc.isRoughnessPerceptual = false; // Not used, all shaders assume roughness is linear
		desc.temporalStabilityFactor = data.TemporalStabilityFactor;
		desc.depthBufferThickness = data.DepthBufferThickness;
		desc.roughnessThreshold = data.RoughnessThreshold;
		desc.varianceThreshold = data.VarianceThreshold;
		desc.maxTraversalIntersections = data.MaxTraversalIntersections;
		desc.minTraversalOccupancy = data.MinTraversalOccupancy;
		desc.mostDetailedMip = data.MostDetailedMip;
		desc.samplesPerQuad = data.SamplesPerQuad;
		desc.temporalVarianceGuidedTracingEnabled = data.TemporalVarianceGuidedTracingEnabled;
		ZE_FFX_THROW_FAILED(ffxSssrContextDispatch(&data.Ctx, &desc), "Error performing SSSR!");

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(void* data) noexcept
	{
		if (ImGui::CollapsingHeader("SSSR##options"))
		{
			ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

			ImGui::Text("Version " ZE_STRINGIFY_VERSION(ZE_DEPAREN(FFX_SSSR_VERSION_MAJOR), ZE_DEPAREN(FFX_SSSR_VERSION_MINOR), ZE_DEPAREN(FFX_SSSR_VERSION_PATCH)));

			ImGui::Text("IBL Factor");
			GUI::InputClamp(0.0f, 1.0f, execData.IblFactor,
				ImGui::InputFloat("##ibl_factor", &execData.IblFactor, 0.01f, 0.1f, "%.2f"));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("A factor to control the intensity of the image based lighting. Set to 1 for an HDR probe.");

			ImGui::Text("Temporal Stability");
			ImGui::SetNextItemWidth(-1.0f);
			GUI::InputClamp(0.0f, 1.0f, execData.TemporalStabilityFactor,
				ImGui::InputFloat("##sssr_temp_stability", &execData.TemporalStabilityFactor, 0.01f, 0.1f, "%.2f"));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("A factor to control the accmulation of history values. Higher values reduce noise, but are more likely to exhibit ghosting artefacts.");

			ImGui::Text("Depth Buffer Thickness");
			ImGui::SetNextItemWidth(-1.0f);
			GUI::InputClamp(0.0f, 0.03f, execData.DepthBufferThickness,
				ImGui::InputFloat("##sssr_depth_thicc", &execData.DepthBufferThickness, 0.001f, 0.01f, "%.3f"));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("A bias for accepting hits. Larger values can cause streaks, lower values can cause holes.");

			ImGui::Text("Roughness Threshold");
			ImGui::SetNextItemWidth(-1.0f);
			GUI::InputClamp(0.0f, 1.0f, execData.RoughnessThreshold,
				ImGui::InputFloat("##sssr_rough_threshold", &execData.RoughnessThreshold, 0.01f, 0.1f, "%.2f"));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Regions with a roughness value greater than this threshold won't spawn rays.");

			ImGui::Text("Temporal Variance Threshold");
			ImGui::SetNextItemWidth(-1.0f);
			GUI::InputClamp(0.0f, 0.01f, execData.VarianceThreshold,
				ImGui::InputFloat("##sssr_variance_threshold", &execData.VarianceThreshold, 0.0001f, 0.001f, "%.4f"));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Luminance differences between history results will trigger an additional ray if they are greater than this threshold value.");

			ImGui::Text("Max Traversal Iterations");
			ImGui::SetNextItemWidth(-1.0f);
			GUI::InputClamp(0U, 256U, execData.MaxTraversalIntersections,
				ImGui::InputInt("##sssr_max_intersect", reinterpret_cast<int*>(&execData.MaxTraversalIntersections), 1, 50));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Caps the maximum number of lookups that are performed from the depth buffer hierarchy. Most rays should terminate after approximately 20 lookups.");

			ImGui::Text("Min Traversal Occupancy");
			ImGui::SetNextItemWidth(-1.0f);
			GUI::InputClamp(0U, 32U, execData.MinTraversalOccupancy,
				ImGui::InputInt("##sssr_min_occupancy", reinterpret_cast<int*>(&execData.MinTraversalOccupancy), 1, 10));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Exit the core loop early if less than this number of threads are running.");

			ImGui::Text("Most Detailed Mip");
			ImGui::SetNextItemWidth(-1.0f);
			GUI::InputClamp(0U, 5U, execData.MostDetailedMip,
				ImGui::InputInt("##sssr_mip_detail", reinterpret_cast<int*>(&execData.MostDetailedMip), 1, 1));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("The most detailed MIP map level in the depth hierarchy. Perfect mirrors always use 0 as the most detailed level.");

			constexpr std::array<const char*, 3> SAMPLES = { "1", "2", "4" };
			U8 sampleIndex = Utils::SafeCast<U8>(execData.SamplesPerQuad == 4 ? 2 : execData.SamplesPerQuad - 1);
			ImGui::SetNextItemWidth(50.0f);
			if (ImGui::BeginCombo("Samples per quad", SAMPLES.at(sampleIndex)))
			{
				for (U8 i = 0; const char* samples : SAMPLES)
				{
					const bool selected = i == sampleIndex;
					if (ImGui::Selectable(samples, selected))
					{
						sampleIndex = i;
						execData.SamplesPerQuad = sampleIndex == 2 ? 4 : sampleIndex + 1;
					}
					if (selected)
						ImGui::SetItemDefaultFocus();
					++i;
				}
				ImGui::EndCombo();
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("The minimum number of rays per quad. Variance guided tracing can increase this up to a maximum of 4.");

			ImGui::Checkbox("Enable Variance Guided Tracing##sssr_enable_variance", &execData.TemporalVarianceGuidedTracingEnabled);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("A boolean controlling whether a ray should be spawned on pixels where a temporal variance is detected or not.");
			ImGui::NewLine();
		}
	}
}