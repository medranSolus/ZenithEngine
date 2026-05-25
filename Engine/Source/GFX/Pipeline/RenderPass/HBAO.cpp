#include "GFX/Pipeline/RenderPass/HBAO.h"
#include "GFX/External/Error.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::HBAO
{
	static Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, PassExecuteData* passData, const std::vector<PixelFormat>& formats) noexcept
	{
		return Update(dev, buildData, *static_cast<ExecuteData*>(passData));
	}

	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, PassInitData* initData) noexcept
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for HBAO initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	PassDesc GetDesc(PixelFormat internalNormalsFormat) noexcept
	{
		PassDesc desc{ Base(CorePassType::HBAO) };
		desc.InitializeFormats.emplace_back(internalNormalsFormat);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.DebugUI = DebugUI;
		return desc;
	}

	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept
	{
		if (passData.RenderSize != Settings::RenderSize)
		{
			passData.RenderSize = Settings::RenderSize;
			ZE_HBAO_LOG_RET_FAILED_EXPECT(passData.Ctx.CreateResources(dev, passData.Params, passData.RenderSize), "Error recreating resources for the HBAO+!");
			return UpdateOperation::InternalOnly;
		}
		return UpdateOperation::NoUpdate;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat internalNormalsFormat) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();

		ZE_EXPECT_RET_FAILED(passData->Ctx, External::HbaoCtx::Create(dev));

		Binding::SchemaDesc desc = {};
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Normals
		desc.AppendSamplers(buildData.Samplers);
		ZE_EXPECT_RET_FAILED(passData->BindingIndex, buildData.BindingLib.AddDataBinding(dev, desc));

		Resource::PipelineStateDesc psoDesc = {};
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache));
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.PS, "UnpackNormalsPS", buildData.ShaderCache));
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = internalNormalsFormat;
		ZE_PSO_SET_NAME(psoDesc, "UnpackNormalsHBAO");

		ZE_EXPECT_RET_FAILED(passData->UnpackNormals, Resource::PipelineStateGfx::Create(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex)));
		
		passData->RenderSize = Settings::RenderSize;
		ZE_HBAO_LOG_RET_FAILED_EXPECT(passData->Ctx.CreateResources(dev, passData->Params, Settings::RenderSize), "Error creating resources for the HBAO+!");

		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		ZE_PERF_GUARD("HBAO");

		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

		ZE_DRAW_TAG_BEGIN(dev, cl, "HBAO", Pixel(0x76, 0xB9, 0x00));

		renderData.Buffers.BeginRaster(cl, ids.InternalNormals);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.UnpackNormals.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.Normal);
		cl.DrawFullscreen(dev);
		renderData.Buffers.EndRaster(cl);

		std::array<BarrierTransition, 2> barriers = {};

		barriers.front().Resource = ids.InternalNormals;
		barriers.front().LayoutBefore = TextureLayout::RenderTarget;
		barriers.front().LayoutAfter = TextureLayout::ShaderResource;
		barriers.front().AccessBefore = Base(ResourceAccess::RenderTarget);
		barriers.front().AccessAfter = Base(ResourceAccess::ShaderResource);
		barriers.front().StageBefore = Base(StageSync::RenderTarget);
		barriers.front().StageAfter = Base(StageSync::PixelShading);
		renderData.Buffers.Barrier(cl, barriers.front());

		ZE_HBAO_LOG_RET_FAILED_EXPECT(data.Ctx.Render(dev, cl, renderData.Buffers, data.Params,
			ids.Depth, ids.InternalNormals, ids.InternalAO,
			renderData.GraphData.Projection, &renderData.DynamicData.ViewTps), "Error performing HBAO+!");

		barriers.front().LayoutBefore = TextureLayout::ShaderResource;
		barriers.front().LayoutAfter = TextureLayout::RenderTarget;
		barriers.front().AccessBefore = Base(ResourceAccess::ShaderResource);
		barriers.front().AccessAfter = Base(ResourceAccess::None);
		barriers.front().StageBefore = Base(StageSync::PixelShading);
		barriers.front().StageAfter = Base(StageSync::None);

		barriers.back().Resource = ids.InternalAO;
		barriers.back().LayoutBefore = TextureLayout::RenderTarget;
		barriers.back().LayoutAfter = TextureLayout::CopySource;
		barriers.back().AccessBefore = Base(ResourceAccess::RenderTarget);
		barriers.back().AccessAfter = Base(ResourceAccess::CopySource);
		barriers.back().StageBefore = Base(StageSync::RenderTarget);
		barriers.back().StageAfter = Base(StageSync::Copy);
		renderData.Buffers.Barrier(cl, barriers);

		renderData.Buffers.CopyFullResource(cl, ids.InternalAO, ids.AO);

		barriers.back().LayoutBefore = TextureLayout::CopySource;
		barriers.back().LayoutAfter = TextureLayout::RenderTarget;
		barriers.back().AccessBefore = Base(ResourceAccess::CopySource);
		barriers.back().AccessAfter = Base(ResourceAccess::None);
		barriers.back().StageBefore = Base(StageSync::Copy);
		barriers.back().StageAfter = Base(StageSync::None);
		renderData.Buffers.Barrier(cl, barriers.back());

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		if (ImGui::CollapsingHeader("HBAO+"))
		{
			ExecuteData& execData = *static_cast<ExecuteData*>(data);

			ImGui::Text("Version %u.%u.%u", GFSDK_SSAO_Version{}.Major, GFSDK_SSAO_Version{}.Minor, GFSDK_SSAO_Version{}.Branch);
			
			ImGui::Columns(2, "##hbao_params", false);
			{
				constexpr std::array<const char*, 2> STEPS = { "4", "8" };
				constexpr std::array<const char*, 2> STEPS_INFO = { "Same as in HBAO+ 3.x", "Slower, to reduce banding artifacts" };
				if (ImGui::BeginCombo("Steps##hbao", STEPS.at(static_cast<U8>(execData.Params.StepCount))))
				{
					for (GFSDK_SSAO_StepCount i = GFSDK_SSAO_STEP_COUNT_4; const char* level : STEPS)
					{
						const bool selected = i == execData.Params.StepCount;
						if (ImGui::Selectable(level, selected))
							execData.Params.StepCount = i;
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip(STEPS_INFO.at(i));
						if (selected)
							ImGui::SetItemDefaultFocus();
						i = static_cast<GFSDK_SSAO_StepCount>(static_cast<U8>(i) + 1);
					}
					ImGui::EndCombo();
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The number of steps per direction in the AO-generation pass");

				ImGui::Text("Radius");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.1f, FLT_MAX, execData.Params.Radius,
					ImGui::InputFloat("##hbao_radius", &execData.Params.Radius, 0.1f, 1.0f, "%.1f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The AO radius in meters");

				ImGui::Text("Small scale AO");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 2.0f, execData.Params.SmallScaleAO,
					ImGui::InputFloat("##hbao_small_scale", &execData.Params.SmallScaleAO, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(" Scale factor for the small-scale AO, the greater the darker");

				ImGui::Checkbox("Foreground AO##hbao_foreground", reinterpret_cast<bool*>(&execData.Params.ForegroundAO.Enable));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("To limit the occlusion scale in the foreground, enabling this may have a small performance impact");

				ImGui::BeginDisabled(!execData.Params.ForegroundAO.Enable);
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, FLT_MAX, execData.Params.ForegroundAO.ForegroundViewDepth,
					ImGui::InputFloat("##hbao_foreground_depth", &execData.Params.ForegroundAO.ForegroundViewDepth, 0.01f, 0.1f, "%.3f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("View-space depth at which the AO footprint should get clamped");
				ImGui::EndDisabled();

				ImGui::Checkbox("Blur##hbao_blur", reinterpret_cast<bool*>(&execData.Params.Blur.Enable));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Optional AO blur, to blur the AO before compositing it");

				ImGui::BeginDisabled(!execData.Params.Blur.Enable);
				{
					constexpr std::array<const char*, 2> RADIUS = { "2 px", "4 px" };
					if (ImGui::BeginCombo("Blur radius##hbao_blur_radius", RADIUS.at(static_cast<U8>(execData.Params.Blur.Radius))))
					{
						for (GFSDK_SSAO_BlurRadius i = GFSDK_SSAO_BLUR_RADIUS_2; const char* level : RADIUS)
						{
							const bool selected = i == execData.Params.Blur.Radius;
							if (ImGui::Selectable(level, selected))
								execData.Params.Blur.Radius = i;
							if (i == GFSDK_SSAO_BLUR_RADIUS_4 && ImGui::IsItemHovered())
								ImGui::SetTooltip("Recommended");
							if (selected)
								ImGui::SetItemDefaultFocus();
							i = static_cast<GFSDK_SSAO_BlurRadius>(static_cast<U8>(i) + 1);
						}
						ImGui::EndCombo();
					}

					ImGui::Text("Blur sharpness");
					ImGui::SetNextItemWidth(-1.0f);
					GUI::InputClamp(0.0f, 16.0f, execData.Params.Blur.Sharpness,
						ImGui::InputFloat("##hbao_blur_sharp", &execData.Params.Blur.Sharpness, 0.1f, 1.0f, "%.1f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("The higher, the more the blur preserves edges");

					ImGui::Checkbox("Blur sharpness profile##hbao_blur_sharp_prof", reinterpret_cast<bool*>(&execData.Params.Blur.SharpnessProfile.Enable));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Optional depth-dependent sharpness function, when enabled, the actual per-pixel blur sharpness value depends on the per-pixel view depth to make the blur sharper in the foreground");
				}
				ImGui::EndDisabled();

				ImGui::BeginDisabled(!execData.Params.Blur.Enable || !execData.Params.Blur.SharpnessProfile.Enable);
				{
					ImGui::Text("Blur background view depth");
					ImGui::SetNextItemWidth(-1.0f);
					GUI::InputClamp(0.0f, FLT_MAX, execData.Params.Blur.SharpnessProfile.BackgroundViewDepth,
						ImGui::InputFloat("##hbao_blur_sharp_background_view", &execData.Params.Blur.SharpnessProfile.BackgroundViewDepth, 0.01f, 0.1f, "%.3f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Minimum view depth of the background depth range");

					ImGui::Text("Blur foreground view depth");
					ImGui::SetNextItemWidth(-1.0f);
					GUI::InputClamp(0.0f, FLT_MAX, execData.Params.Blur.SharpnessProfile.ForegroundViewDepth,
						ImGui::InputFloat("##hbao_blur_sharp_foreground_view", &execData.Params.Blur.SharpnessProfile.ForegroundViewDepth, 0.01f, 0.1f, "%.3f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Maximum view depth of the foreground depth range");

					ImGui::Text("Blur foreground sharpness");
					ImGui::SetNextItemWidth(-1.0f);
					GUI::InputClamp(0.0f, FLT_MAX, execData.Params.Blur.SharpnessProfile.ForegroundSharpnessScale,
						ImGui::InputFloat("##hbao_blur_sharp_foreground", &execData.Params.Blur.SharpnessProfile.ForegroundSharpnessScale, 0.1f, 1.0f, "%.1f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Sharpness scale factor for ViewDepths <= ForegroundViewDepth");
				}
				ImGui::EndDisabled();
			}
			ImGui::NextColumn();
			{
				constexpr std::array<const char*, 2> CLAMP = { "To edge", "To border" };
				constexpr std::array<const char*, 2> CLAMP_INFO = { "May cause false occlusion near screen borders", "May cause halos near screen borders" };
				if (ImGui::BeginCombo("Clamp##hbao", CLAMP.at(static_cast<U8>(execData.Params.DepthClampMode))))
				{
					for (GFSDK_SSAO_DepthClampMode i = GFSDK_SSAO_CLAMP_TO_EDGE; const char* level : CLAMP)
					{
						const bool selected = i == execData.Params.DepthClampMode;
						if (ImGui::Selectable(level, selected))
							execData.Params.DepthClampMode = i;
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip(CLAMP_INFO.at(i));
						if (selected)
							ImGui::SetItemDefaultFocus();
						i = static_cast<GFSDK_SSAO_DepthClampMode>(static_cast<U8>(i) + 1);
					}
					ImGui::EndCombo();
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("To hide possible false-occlusion artifacts near screen borders");

				ImGui::Text("Bias");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 0.5f, execData.Params.Bias,
					ImGui::InputFloat("##hbao_bias", &execData.Params.Bias, 0.01f, 0.1f, "%.3f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("To hide low-tessellation artifacts");

				ImGui::Text("Large scale AO");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 2.0f, execData.Params.LargeScaleAO,
					ImGui::InputFloat("##hbao_large_scale", &execData.Params.LargeScaleAO, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Scale factor for the large-scale AO, the greater the darker");

				ImGui::Checkbox("Background AO##hbao_background", reinterpret_cast<bool*>(&execData.Params.BackgroundAO.Enable));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("To add larger-scale occlusion in the distance, enabling this may have a small performance impact");

				ImGui::BeginDisabled(!execData.Params.BackgroundAO.Enable);
				GUI::InputClamp(0.0f, FLT_MAX, execData.Params.BackgroundAO.BackgroundViewDepth,
					ImGui::InputFloat("##hbao_background_depth", &execData.Params.BackgroundAO.BackgroundViewDepth, 0.01f, 0.1f, "%.3f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("View-space depth at which the AO footprint should stop falling off with depth");
				ImGui::EndDisabled();

				ImGui::Checkbox("Dual-layer AO##hbao_dual_layer", reinterpret_cast<bool*>(&execData.Params.EnableDualLayerAO));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("To reduce halo artifacts behind foreground object");

				constexpr std::array<const char*, 2> DEPTH = { "FP16", "FP32" };
				constexpr std::array<const char*, 2> DEPTH_INFO = { "Recommended", "Slower" };
				if (ImGui::BeginCombo("Depth storage##hbao", DEPTH.at(static_cast<U8>(execData.Params.DepthStorage))))
				{
					for (GFSDK_SSAO_DepthStorage i = GFSDK_SSAO_FP16_VIEW_DEPTHS; const char* level : DEPTH)
					{
						const bool selected = i == execData.Params.DepthStorage;
						if (ImGui::Selectable(level, selected))
							execData.Params.DepthStorage = i;
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip(DEPTH_INFO.at(i));
						if (selected)
							ImGui::SetItemDefaultFocus();
						i = static_cast<GFSDK_SSAO_DepthStorage>(static_cast<U8>(i) + 1);
					}
					ImGui::EndCombo();
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The format of the internal depth texture sampled in the AO-generation pass");

				ImGui::Text("Power");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(1.0f, 4.0f, execData.Params.PowerExponent,
					ImGui::InputFloat("##hbao_power", &execData.Params.PowerExponent, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The final AO output is pow(AO, powerExponent)");

				ImGui::Checkbox("Depth treshold##hbao_depth_treshold", reinterpret_cast<bool*>(&execData.Params.DepthThreshold.Enable));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Optional Z threshold, to hide possible depth-precision artifacts");

				ImGui::BeginDisabled(!execData.Params.DepthThreshold.Enable);
				{
					ImGui::Text("Max view depth treshold");
					ImGui::SetNextItemWidth(-1.0f);
					GUI::InputClamp(0.0f, FLT_MAX, execData.Params.DepthThreshold.MaxViewDepth,
						ImGui::InputFloat("##hbao_depth_treshold_max", &execData.Params.DepthThreshold.MaxViewDepth, 0.01f, 0.1f, "%.3f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("To return white AO for ViewDepths > MaxViewDepth");

					ImGui::Text("Depth treshold sharpness");
					ImGui::SetNextItemWidth(-1.0f);
					GUI::InputClamp(0.0f, FLT_MAX, execData.Params.DepthThreshold.Sharpness,
						ImGui::InputFloat("##hbao_depth_treshold_sharpness", &execData.Params.DepthThreshold.Sharpness, 1.0f, 10.0f, "%.1f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("The higher, the sharper are the AO-to-white transitions");
				}
				ImGui::EndDisabled();

			}
			ImGui::Columns(1);

			ImGui::NewLine();
		}
	}
}