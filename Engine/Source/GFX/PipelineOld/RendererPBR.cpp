#include "GFX/PipelineOld/RendererPBR.h"

#define ZE_MAKE_NODE(name, queueType, passNamespace, ...) RenderNode node(name, queueType, RenderPass::passNamespace::Execute, RenderPass::passNamespace::Clean, RenderPass::passNamespace::Setup(__VA_ARGS__))

namespace ZE::GFX::Pipeline
{/*
	void RendererPBR::UpdateSettingsData(const Data::Projection& projection) noexcept
	{
		currentProjectionData = projection;
		// No need to create new projection as it's data is always changing with jitter
		if (!Settings::ApplyJitter())
			Math::XMStoreFloat4x4(&currentProjection, Data::GetProjectionMatrix(projection));
		dynamicData.JitterCurrent = { 0.0f, 0.0f };
		// Not uploading now since it's uploaded every frame
		dynamicData.NearClip = currentProjectionData.NearClip;
	}

	void RendererPBR::SetInverseViewProjection(EID camera) noexcept
	{
		Data::Camera& camData = Settings::Data.get<Data::Camera>(camera);
		UpdateSettingsData(camData.Projection);

		const auto& transform = Settings::Data.get<Data::Transform>(camera); // TODO: Change into TransformGlobal later
		Math::XMStoreFloat4x4(&dynamicData.ViewProjectionInverseTps, Math::XMMatrixTranspose(Math::XMMatrixInverse(nullptr,
			Math::XMMatrixLookToLH(Math::XMLoadFloat3(&transform.Position),
				Math::XMLoadFloat3(&camData.EyeDirection),
				Math::XMLoadFloat3(&camData.UpVector)) * Data::GetProjectionMatrix(camData.Projection))));

		if (Settings::ApplyJitter())
			CalculateJitter(jitterIndex, dynamicData.JitterCurrent.x, dynamicData.JitterCurrent.y, Settings::RenderSize, Settings::GetUpscaler());
	}

	void RendererPBR::UpdateWorldData(Device& dev, EID camera) noexcept
	{
		ZE_ASSERT((Settings::Data.all_of<Data::TransformGlobal, Data::Camera>(camera)),
			"Current camera does not have all required components!");

		const auto& currentCamera = Settings::Data.get<Data::Camera>(camera);
		const auto& transform = Settings::Data.get<Data::Transform>(camera); // TODO: Change into TransformGlobal later
		cameraRotation = transform.Rotation;

		// Setup shader world data
		dynamicData.CameraPos = transform.Position;
		const Matrix view = Math::XMMatrixLookToLH(Math::XMLoadFloat3(&dynamicData.CameraPos),
			Math::XMLoadFloat3(&currentCamera.EyeDirection),
			Math::XMLoadFloat3(&currentCamera.UpVector));
		Math::XMStoreFloat4x4(&dynamicData.ViewTps, Math::XMMatrixTranspose(view));

		if (Settings::ComputeMotionVectors())
			prevViewProjectionTps = dynamicData.ViewProjectionTps;

		Matrix projection;
		if (Settings::ApplyJitter())
		{
			CalculateJitter(jitterIndex, currentProjectionData.JitterX, currentProjectionData.JitterY, Settings::RenderSize, Settings::GetUpscaler());
			dynamicData.JitterPrev = dynamicData.JitterCurrent;
			dynamicData.JitterCurrent = { currentProjectionData.JitterX, currentProjectionData.JitterY };

			projection = Data::GetProjectionMatrix(currentProjectionData);
			Math::XMStoreFloat4x4(&currentProjection, projection);
		}
		else
			projection = Math::XMLoadFloat4x4(&currentProjection);

		const Matrix viewProjection = view * projection;
		Math::XMStoreFloat4x4(&dynamicData.ViewProjectionTps, Math::XMMatrixTranspose(viewProjection));
		Math::XMStoreFloat4x4(&dynamicData.ViewProjectionInverseTps, Math::XMMatrixTranspose(Math::XMMatrixInverse(nullptr, viewProjection)));
	}

	void RendererPBR::ShowWindow(Device& dev, Data::AssetsStreamer& assets)
	{
		bool change = false;
		if (ImGui::CollapsingHeader("Outline"))
		{
			ImGui::Columns(2, "##outline_options", false);
			{
				ImGui::Text("Blur radius");
				ImGui::SetNextItemWidth(-1.0f);
				change |= ImGui::SliderInt("##blur_radius", &settingsData.BlurRadius, 1, DataPBR::BLUR_KERNEL_RADIUS);
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Outline range");
				ImGui::SetNextItemWidth(-1.0f);
				if (GUI::InputClamp(0.1f, 25.0f, blurSigma,
					ImGui::InputFloat("##blur_sigma", &blurSigma, 0.1f, 0.0f, "%.1f")))
				{
					change = true;
					SetupBlurKernel();
				}
			}
			ImGui::Columns(1);
			ImGui::NewLine();
		}
		if (ImGui::CollapsingHeader("Display"))
		{
			ImGui::Columns(2, "##display_options", false);
			{
				ImGui::Text("Gamma correction");
				ImGui::SetNextItemWidth(-1.0f);
				if (GUI::InputClamp(1.0f, 10.0f, settingsData.Gamma,
					ImGui::InputFloat("##gamma", &settingsData.Gamma, 0.1f, 0.0f, "%.1f")))
				{
					change = true;
					settingsData.GammaInverse = 1.0f / settingsData.Gamma;
				}
			}
			ImGui::NextColumn();
			{
				ImGui::Text("HDR exposure");
				ImGui::SetNextItemWidth(-1.0f);
				if (GUI::InputClamp(0.1f, FLT_MAX, settingsData.HDRExposure,
					ImGui::InputFloat("##hdr", &settingsData.HDRExposure, 0.1f, 0.0f, "%.1f")))
				{
					change = true;
					SetupBlurIntensity();
				}
			}
			ImGui::Columns(1);
			ImGui::NewLine();
		}
		if (ImGui::CollapsingHeader("Shadows"))
		{
			ImGui::Columns(2, "##shadow_options", false);
			{
				ImGui::Text("Depth bias");
				ImGui::SetNextItemWidth(-1.0f);
				S32 bias = Utils::SafeCast<S32>(settingsData.ShadowBias * settingsData.ShadowMapSize);
				if (ImGui::InputInt("##depth_bias", &bias))
				{
					change = true;
					settingsData.ShadowBias = Utils::SafeCast<float>(bias) / settingsData.ShadowMapSize;
				}
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Normal offset");
				ImGui::SetNextItemWidth(-1.0f);
				change |= GUI::InputClamp(0.0f, 1.0f, settingsData.ShadowNormalOffset,
					ImGui::InputFloat("##normal_offset", &settingsData.ShadowNormalOffset, 0.001f, 0.0f, "%.3f"));
			}
			ImGui::Columns(1);

			ImGui::Text("Ambient color");
			ImGui::SetNextItemWidth(-5.0f);
			change |= ImGui::ColorEdit3("##ambient_color", reinterpret_cast<float*>(&settingsData.AmbientLight),
				ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoLabel);
			ImGui::NewLine();
		}
		switch (Settings::GetAOType())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case AOType::None:
			break;
		case AOType::XeGTAO:
		{
			if (ImGui::CollapsingHeader("XeGTAO"))
			{
				// GTAOImGuiSettings() don't indicate if quality or denoise passes has been updated...
				const int quality = ssaoSettings.xegtao.Settings.QualityLevel;
				const int denoise = ssaoSettings.xegtao.Settings.DenoisePasses;
				XeGTAO::GTAOImGuiSettings(ssaoSettings.xegtao.Settings);
				change |= quality != ssaoSettings.xegtao.Settings.QualityLevel || denoise != ssaoSettings.xegtao.Settings.DenoisePasses;
				if (change)
					SetupXeGTAOQuality();
				ImGui::NewLine();
			}
			break;
		}
		case AOType::CACAO:
		{
			if (ImGui::CollapsingHeader("CACAO"))
			{
				constexpr std::array<const char*, 5> LEVELS = { "Lowest", "Low", "Medium", "High", "Highest" };
				if (ImGui::BeginCombo("Quality level", LEVELS.at(ssaoSettings.cacao.qualityLevel)))
				{
					for (FfxCacaoQuality i = FFX_CACAO_QUALITY_LOWEST; const char* level : LEVELS)
					{
						const bool selected = i == ssaoSettings.cacao.qualityLevel;
						if (ImGui::Selectable(level, selected))
							ssaoSettings.cacao.qualityLevel = i;
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
					GUI::InputClamp(0.1f, FLT_MAX, ssaoSettings.cacao.radius,
						ImGui::InputFloat("##cacao_blur_radius", &ssaoSettings.cacao.radius, 0.1f, 1.0f, "%.1f"));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Size of the occlusion sphere");
				}
				ImGui::NextColumn();
				{
					ImGui::Text("Blur pass count");
					ImGui::SetNextItemWidth(-1.0f);
					ImGui::SliderInt("##cacao_blur_count", reinterpret_cast<int*>(&ssaoSettings.cacao.blurPassCount), 0, 8);
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Number of edge-sensitive smart blur passes to apply");
				}
				ImGui::Columns(1);

				if (ssaoSettings.cacao.qualityLevel != FFX_CACAO_QUALITY_HIGHEST)
					ImGui::BeginDisabled(true);
				ImGui::Text("Adaptative quality limit");
				ImGui::InputFloat("##cacao_adapt_limit", &ssaoSettings.cacao.adaptiveQualityLimit, 0.01f, 0.1f, "%.2f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Only for highest quality level");
				if (ssaoSettings.cacao.qualityLevel != FFX_CACAO_QUALITY_HIGHEST)
					ImGui::EndDisabled();

				if (ImGui::CollapsingHeader("Advanced"))
				{
					ImGui::Columns(2, "##cacao_advanced", false);
					{
						ImGui::Text("Sharpness");
						GUI::InputClamp(0.0f, 1.0f, ssaoSettings.cacao.sharpness,
							ImGui::InputFloat("##cacao_sharpness", &ssaoSettings.cacao.sharpness, 0.01f, 0.1f, "%.2f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("How much to bleed over edges; 1: not at all, 0.5: half-half; 0.0: completely ignore edges");
						ImGui::Text("Fade out start");
						GUI::InputClamp(0.0f, ssaoSettings.cacao.fadeOutTo, ssaoSettings.cacao.fadeOutFrom,
							ImGui::InputFloat("##cacao_fade_from", &ssaoSettings.cacao.fadeOutFrom, 1.0f, 5.0f, "%.1f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Distance to start fading out the effect");
					}
					ImGui::NextColumn();
					{
						ImGui::Text("Horizontal angle treshold");
						GUI::InputClamp(0.0f, 0.2f, ssaoSettings.cacao.horizonAngleThreshold,
							ImGui::InputFloat("##cacao_horizon_angle", &ssaoSettings.cacao.horizonAngleThreshold, 0.01f, 0.0f, "%.2f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Limits self-shadowing (makes the sampling area less of a hemisphere, more of a spherical cone, to avoid self-shadowing and various artifacts due to low tessellation and depth buffer imprecision, etc.)");
						ImGui::Text("Fade out end");
						GUI::InputClamp(ssaoSettings.cacao.fadeOutFrom, FLT_MAX, ssaoSettings.cacao.fadeOutTo,
							ImGui::InputFloat("##cacao_fade_to", &ssaoSettings.cacao.fadeOutTo, 1.0f, 5.0f, "%.1f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Distance at which the effect is faded out");
					}
					ImGui::Columns(1);

					ImGui::Text("Shadow controls");
					ImGui::Columns(2, "##cacao_shadows", false);
					{
						ImGui::Text("Shadow multipler");
						GUI::InputClamp(0.0f, 5.0f, ssaoSettings.cacao.shadowMultiplier,
							ImGui::InputFloat("##cacao_shadow_mult", &ssaoSettings.cacao.shadowMultiplier, 0.1f, 1.0f, "%.1f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Effect strength linear multiplier");
						ImGui::Text("Shadow clamp");
						GUI::InputClamp(0.0f, 1.0f, ssaoSettings.cacao.shadowClamp,
							ImGui::InputFloat("##cacao_shadow_clamp", &ssaoSettings.cacao.shadowClamp, 0.01f, 0.1f, "%.2f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Effect max limit (applied after multiplier but before blur)");
					}
					ImGui::NextColumn();
					{
						ImGui::Text("Shadow power");
						GUI::InputClamp(0.5f, 5.0f, ssaoSettings.cacao.shadowPower,
							ImGui::InputFloat("##cacao_shadow_pow", &ssaoSettings.cacao.shadowPower, 0.01f, 0.1f, "%.2f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Effect strength modifier");
						ImGui::Text("Shadow detail");
						GUI::InputClamp(0.5f, 5.0f, ssaoSettings.cacao.detailShadowStrength,
							ImGui::InputFloat("##cacao_shadow_det", &ssaoSettings.cacao.detailShadowStrength, 0.1f, 1.0f, "%.1f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Used for high-res detail AO using neighboring depth pixels: adds a lot of detail but also reduces temporal stability (adds aliasing)");
					}
					ImGui::Columns(1);

					ImGui::Text("Bilateral sigma");
					ImGui::Columns(2, "##cacao_bilateral", false);
					{
						ImGui::Text("Blur term");
						GUI::InputClamp(0.0f, FLT_MAX, ssaoSettings.cacao.bilateralSigmaSquared,
							ImGui::InputFloat("##cacao_bil_sigma", &ssaoSettings.cacao.bilateralSigmaSquared, 0.1f, 1.0f, "%.1f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Sigma squared value for use in bilateral upsampler giving Gaussian blur term");
					}
					ImGui::NextColumn();
					{
						ImGui::Text("Similarity weight");
						GUI::InputClamp(0.0f, FLT_MAX, ssaoSettings.cacao.bilateralSimilarityDistanceSigma,
							ImGui::InputFloat("##cacao_bil_similarity", &ssaoSettings.cacao.bilateralSimilarityDistanceSigma, 0.01f, 0.1f, "%.2f"));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Sigma squared value for use in bilateral upsampler giving similarity weighting for neighbouring pixels");
					}
					ImGui::Columns(1);
					ImGui::NewLine();
				}
			}
			break;
		}
		}
		switch (Settings::GetUpscaler())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case UpscalerType::None:
		case UpscalerType::XeSS: // At the moment no options for XeSS since only quality mode can be chosen
			break;
		case UpscalerType::Fsr1:
		case UpscalerType::Fsr2:
		case UpscalerType::NIS:
		{
			const bool fsr2 = Settings::GetUpscaler() == UpscalerType::Fsr2;
			const bool nis = Settings::GetUpscaler() == UpscalerType::NIS;
			if (ImGui::CollapsingHeader(nis ? "NIS" : (fsr2 ? "FSR2" : "FSR1")))
			{
				ImGui::Columns(2, "##sharpness_settings", false);
				{
					ImGui::Text("Sharpness");
				}
				ImGui::NextColumn();
				{
					ImGui::Checkbox("##enable_sharpness", &enableSharpening);
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Enable an additional sharpening pass");
				}
				ImGui::Columns(1);

				if (!enableSharpening)
					ImGui::BeginDisabled(true);
				GUI::InputClamp(0.0f, 1.0f, sharpness,
					ImGui::InputFloat("##fsr_sharpness", &sharpness, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The sharpness value between 0 and 1, where 0 is no additional sharpness and 1 is maximum additional sharpness");
				if (!enableSharpening)
					ImGui::EndDisabled();
				ImGui::NewLine();
			}
			break;
		}
		}
		if (Settings::IsEnabledSSSR())
		{
			if (ImGui::CollapsingHeader("SSSR"))
			{
				GUI::InputClamp(0.0f, 1.0f, iblFactor,
					ImGui::InputFloat("##ibl_factor", &iblFactor, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("A factor to control the intensity of the image based lighting. Set to 1 for an HDR probe.");

				ImGui::Text("Temporal Stability");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 1.0f, sssrSettings.TemporalStabilityFactor,
					ImGui::InputFloat("##sssr_temp_stability", &sssrSettings.TemporalStabilityFactor, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("A factor to control the accmulation of history values. Higher values reduce noise, but are more likely to exhibit ghosting artefacts.");

				ImGui::Text("Depth Buffer Thickness");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 0.03f, sssrSettings.DepthBufferThickness,
					ImGui::InputFloat("##sssr_depth_thicc", &sssrSettings.DepthBufferThickness, 0.001f, 0.01f, "%.3f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("A bias for accepting hits. Larger values can cause streaks, lower values can cause holes.");

				ImGui::Text("Roughness Threshold");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 1.0f, sssrSettings.RoughnessThreshold,
					ImGui::InputFloat("##sssr_rough_threshold", &sssrSettings.RoughnessThreshold, 0.01f, 0.1f, "%.2f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Regions with a roughness value greater than this threshold won't spawn rays.");

				ImGui::Text("Temporal Variance Threshold");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0.0f, 0.01f, sssrSettings.VarianceThreshold,
					ImGui::InputFloat("##sssr_variance_threshold", &sssrSettings.VarianceThreshold, 0.0001f, 0.001f, "%.4f"));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Luminance differences between history results will trigger an additional ray if they are greater than this threshold value.");

				ImGui::Text("Max Traversal Iterations");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0U, 256U, sssrSettings.MaxTraversalIntersections,
					ImGui::InputInt("##sssr_max_intersect", reinterpret_cast<int*>(&sssrSettings.MaxTraversalIntersections), 1, 50));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Caps the maximum number of lookups that are performed from the depth buffer hierarchy. Most rays should terminate after approximately 20 lookups.");

				ImGui::Text("Min Traversal Occupancy");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0U, 32U, sssrSettings.MinTraversalOccupancy,
					ImGui::InputInt("##sssr_min_occupancy", reinterpret_cast<int*>(&sssrSettings.MinTraversalOccupancy), 1, 10));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Exit the core loop early if less than this number of threads are running.");

				ImGui::Text("Most Detailed Mip");
				ImGui::SetNextItemWidth(-1.0f);
				GUI::InputClamp(0U, 5U, sssrSettings.MostDetailedMip,
					ImGui::InputInt("##sssr_mip_detail", reinterpret_cast<int*>(&sssrSettings.MostDetailedMip), 1, 1));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The most detailed MIP map level in the depth hierarchy. Perfect mirrors always use 0 as the most detailed level.");

				constexpr std::array<const char*, 3> SAMPLES = { "1", "2", "4" };
				U8 sampleIndex = Utils::SafeCast<U8>(sssrSettings.SamplesPerQuad == 4 ? 2 : sssrSettings.SamplesPerQuad - 1);
				ImGui::SetNextItemWidth(50.0f);
				if (ImGui::BeginCombo("Samples per quad", SAMPLES.at(sampleIndex)))
				{
					for (U8 i = 0; const char* samples : SAMPLES)
					{
						const bool selected = i == sampleIndex;
						if (ImGui::Selectable(samples, selected))
						{
							sampleIndex = i;
							sssrSettings.SamplesPerQuad = sampleIndex == 2 ? 4 : sampleIndex + 1;
						}
						if (selected)
							ImGui::SetItemDefaultFocus();
						++i;
					}
					ImGui::EndCombo();
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The minimum number of rays per quad. Variance guided tracing can increase this up to a maximum of 4.");

				ImGui::Checkbox("Enable Variance Guided Tracing##sssr_enable_variance", &sssrSettings.TemporalVarianceGuidedTracingEnabled);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("A boolean controlling whether a ray should be spawned on pixels where a temporal variance is detected or not.");
				ImGui::NewLine();
			}
		}
		// If any settings data updated then upload new buffer
		if (change)
			execData.SettingsBuffer.Update(dev, assets.GetDisk(), { INVALID_EID, &settingsData, nullptr, sizeof(DataPBR) });
	}
	*/
}