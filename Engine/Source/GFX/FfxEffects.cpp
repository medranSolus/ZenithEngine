#include "GFX/FfxEffects.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_cacao.h"
#include "FidelityFX/host/ffx_denoiser.h"
#include "FidelityFX/host/ffx_fsr1.h"
#include "FidelityFX/host/ffx_fsr2.h"
#include "FidelityFX/host/ffx_fsr3upscaler.h"
#include "FidelityFX/host/ffx_sssr.h"
#include "../src/components/cacao/ffx_cacao_private.h"
#include "../src/components/denoiser/ffx_denoiser_private.h"
#include "../src/components/fsr1/ffx_fsr1_private.h"
#include "../src/components/fsr2/ffx_fsr2_private.h"
#include "../src/components/fsr3upscaler/ffx_fsr3upscaler_private.h"
#include "../src/components/sssr/ffx_sssr_private.h"
ZE_WARNING_POP

namespace ZE::GFX::FFX
{
	FfxErrorCode GetShaderInfoCACAO(Device* dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader);
	FfxErrorCode GetShaderInfoDenoiser(Device* dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader);
	FfxErrorCode GetShaderInfoFSR1(Device* dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader);
	FfxErrorCode GetShaderInfoFSR2(Device* dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader);
	FfxErrorCode GetShaderInfoFSR3(Device* dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader);
	FfxErrorCode GetShaderInfoSSSR(Device* dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader);
	std::string GetGeneralPermutation(bool fp16, bool wave64) noexcept;

	U64 GetPipelineID(FfxEffect effect, FfxPass passId, U32 permutationOptions) noexcept
	{
		ZE_ASSERT(effect <= UINT8_MAX, "FFX effect id outside of cast range!");
		ZE_ASSERT(passId <= UINT8_MAX, "FFX pass id outside of cast range!");
		switch (effect)
		{
		case FFX_EFFECT_CACAO:
		{
			switch (passId)
			{
			case FFX_CACAO_PASS_CLEAR_LOAD_COUNTER:
				permutationOptions = 0;
				break;
			case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_DEPTHS:
			case FFX_CACAO_PASS_PREPARE_NATIVE_DEPTHS:
			case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_DEPTHS_HALF:
			case FFX_CACAO_PASS_PREPARE_NATIVE_DEPTHS_HALF:
			case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_DEPTHS_AND_MIPS:
			case FFX_CACAO_PASS_PREPARE_NATIVE_DEPTHS_AND_MIPS:
			case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_NORMALS:
			case FFX_CACAO_PASS_PREPARE_NATIVE_NORMALS:
			case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_NORMALS_FROM_INPUT_NORMALS:
			case FFX_CACAO_PASS_PREPARE_NATIVE_NORMALS_FROM_INPUT_NORMALS:
			case FFX_CACAO_PASS_GENERATE_Q0:
			case FFX_CACAO_PASS_GENERATE_Q1:
			case FFX_CACAO_PASS_GENERATE_Q2:
			case FFX_CACAO_PASS_GENERATE_Q3:
			case FFX_CACAO_PASS_GENERATE_Q3_BASE:
			case FFX_CACAO_PASS_GENERATE_IMPORTANCE_MAP:
			case FFX_CACAO_PASS_POST_PROCESS_IMPORTANCE_MAP_A:
			case FFX_CACAO_PASS_POST_PROCESS_IMPORTANCE_MAP_B:
			case FFX_CACAO_PASS_APPLY_NON_SMART_HALF:
			case FFX_CACAO_PASS_APPLY_NON_SMART:
			case FFX_CACAO_PASS_APPLY:
				permutationOptions &= CACAO_SHADER_PERMUTATION_FORCE_WAVE64;
				break;
			case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_1:
			case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_2:
			case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_3:
			case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_4:
			case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_5:
			case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_6:
			case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_7:
			case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_8:
				permutationOptions &= CACAO_SHADER_PERMUTATION_FORCE_WAVE64 | CACAO_SHADER_PERMUTATION_ALLOW_FP16;
				break;
			case FFX_CACAO_PASS_UPSCALE_BILATERAL_5X5:
				break;
			default:
				ZE_FAIL("Invalid pass for CACAO!");
				break;
			}
			break;
		}
		case FFX_EFFECT_DENOISER:
		{
			switch (passId)
			{
			case FFX_DENOISER_PASS_PREPARE_SHADOW_MASK:
			case FFX_DENOISER_PASS_SHADOWS_TILE_CLASSIFICATION:
			case FFX_DENOISER_PASS_FILTER_SOFT_SHADOWS_0:
			case FFX_DENOISER_PASS_FILTER_SOFT_SHADOWS_1:
			case FFX_DENOISER_PASS_FILTER_SOFT_SHADOWS_2:
			case FFX_DENOISER_PASS_REPROJECT_REFLECTIONS:
			case FFX_DENOISER_PASS_PREFILTER_REFLECTIONS:
			case FFX_DENOISER_PASS_RESOLVE_TEMPORAL_REFLECTIONS:
				permutationOptions &= DENOISER_SHADER_PERMUTATION_FORCE_WAVE64 | DENOISER_SHADER_PERMUTATION_ALLOW_FP16;
				break;
			default:
				ZE_FAIL("Invalid pass for Denoiser!");
				break;
			}
			break;
		}
		case FFX_EFFECT_FSR1:
		{
			switch (passId)
			{
			case FFX_FSR1_PASS_EASU:
			case FFX_FSR1_PASS_EASU_RCAS:
				permutationOptions &= FSR1_SHADER_PERMUTATION_SRGB_CONVERSIONS
					| FSR1_SHADER_PERMUTATION_FORCE_WAVE64 | FSR1_SHADER_PERMUTATION_ALLOW_FP16;
				break;
			case FFX_FSR1_PASS_RCAS:
				permutationOptions &= FSR1_SHADER_PERMUTATION_RCAS_PASSTHROUGH_ALPHA
					| FSR1_SHADER_PERMUTATION_FORCE_WAVE64 | FSR1_SHADER_PERMUTATION_ALLOW_FP16;
				break;
			default:
				ZE_FAIL("Invalid pass for FSR1!");
				break;
			}
			break;
		}
		case FFX_EFFECT_FSR2:
		{
			switch (passId)
			{
			case FFX_FSR2_PASS_DEPTH_CLIP:
				permutationOptions &= FSR2_SHADER_PERMUTATION_FORCE_WAVE64 | FSR2_SHADER_PERMUTATION_ALLOW_FP16 | FSR2_SHADER_PERMUTATION_DEPTH_INVERTED
					| FSR2_SHADER_PERMUTATION_LOW_RES_MOTION_VECTORS | FSR2_SHADER_PERMUTATION_JITTER_MOTION_VECTORS;
				break;
			case FFX_FSR2_PASS_RECONSTRUCT_PREVIOUS_DEPTH:
				permutationOptions &= FSR2_SHADER_PERMUTATION_FORCE_WAVE64 | FSR2_SHADER_PERMUTATION_ALLOW_FP16 | FSR2_SHADER_PERMUTATION_HDR_COLOR_INPUT
					| FSR2_SHADER_PERMUTATION_LOW_RES_MOTION_VECTORS | FSR2_SHADER_PERMUTATION_JITTER_MOTION_VECTORS;
				break;
			case FFX_FSR2_PASS_LOCK:
				permutationOptions &= FSR2_SHADER_PERMUTATION_FORCE_WAVE64 | FSR2_SHADER_PERMUTATION_ALLOW_FP16 | FSR2_SHADER_PERMUTATION_DEPTH_INVERTED;
				break;
			case FFX_FSR2_PASS_ACCUMULATE:
			case FFX_FSR2_PASS_ACCUMULATE_SHARPEN:
				permutationOptions &= FSR2_SHADER_PERMUTATION_FORCE_WAVE64 | FSR2_SHADER_PERMUTATION_ALLOW_FP16
					| FSR2_SHADER_PERMUTATION_HDR_COLOR_INPUT | FSR2_SHADER_PERMUTATION_LOW_RES_MOTION_VECTORS
					| FSR2_SHADER_PERMUTATION_USE_LANCZOS_TYPE | FSR2_SHADER_PERMUTATION_JITTER_MOTION_VECTORS;
				break;
			case FFX_FSR2_PASS_COMPUTE_LUMINANCE_PYRAMID:
				permutationOptions &= FSR2_SHADER_PERMUTATION_FORCE_WAVE64;
				break;
			case FFX_FSR2_PASS_RCAS:
				permutationOptions &= FSR2_SHADER_PERMUTATION_FORCE_WAVE64 | (_ZE_PLATFORM_XBOX_SCARLET ? FSR2_SHADER_PERMUTATION_ALLOW_FP16 : 0);
				break;
			case FFX_FSR2_PASS_GENERATE_REACTIVE:
				permutationOptions &= FSR2_SHADER_PERMUTATION_FORCE_WAVE64 | FSR2_SHADER_PERMUTATION_ALLOW_FP16;
				break;
			case FFX_FSR2_PASS_TCR_AUTOGENERATE:
				permutationOptions &= FSR2_SHADER_PERMUTATION_FORCE_WAVE64 | FSR2_SHADER_PERMUTATION_ALLOW_FP16 | FSR2_SHADER_PERMUTATION_JITTER_MOTION_VECTORS;
				break;
			default:
				ZE_FAIL("Invalid pass for FSR2!");
				break;
			}
			break;
		}
		case FFX_EFFECT_FSR3UPSCALER:
		{
			switch (passId)
			{
			case FFX_FSR3UPSCALER_PASS_PREPARE_INPUTS:
				permutationOptions &= FSR3UPSCALER_SHADER_PERMUTATION_FORCE_WAVE64 | FSR3UPSCALER_SHADER_PERMUTATION_DEPTH_INVERTED
					| FSR3UPSCALER_SHADER_PERMUTATION_LOW_RES_MOTION_VECTORS | FSR3UPSCALER_SHADER_PERMUTATION_JITTER_MOTION_VECTORS;
				break;
			case FFX_FSR3UPSCALER_PASS_LUMA_PYRAMID:
			case FFX_FSR3UPSCALER_PASS_SHADING_CHANGE_PYRAMID:
			case FFX_FSR3UPSCALER_PASS_SHADING_CHANGE:
			case FFX_FSR3UPSCALER_PASS_LUMA_INSTABILITY:
			case FFX_FSR3UPSCALER_PASS_DEBUG_VIEW:
				permutationOptions &= FSR3UPSCALER_SHADER_PERMUTATION_FORCE_WAVE64;
				break;
			case FFX_FSR3UPSCALER_PASS_PREPARE_REACTIVITY:
			case FFX_FSR3UPSCALER_PASS_GENERATE_REACTIVE:
				permutationOptions &= FSR3UPSCALER_SHADER_PERMUTATION_FORCE_WAVE64 | FSR3UPSCALER_SHADER_PERMUTATION_ALLOW_FP16;
				break;
			case FFX_FSR3UPSCALER_PASS_ACCUMULATE:
			case FFX_FSR3UPSCALER_PASS_ACCUMULATE_SHARPEN:
				permutationOptions &= FSR3UPSCALER_SHADER_PERMUTATION_FORCE_WAVE64
					| FSR3UPSCALER_SHADER_PERMUTATION_ALLOW_FP16 | FSR3UPSCALER_SHADER_PERMUTATION_USE_LANCZOS_TYPE
					| FSR3UPSCALER_SHADER_PERMUTATION_ENABLE_SHARPENING | FSR3UPSCALER_SHADER_PERMUTATION_HDR_COLOR_INPUT
					| FSR3UPSCALER_SHADER_PERMUTATION_LOW_RES_MOTION_VECTORS | FSR3UPSCALER_SHADER_PERMUTATION_JITTER_MOTION_VECTORS;
				break;
			case FFX_FSR3UPSCALER_PASS_RCAS:
				permutationOptions &= FSR3UPSCALER_SHADER_PERMUTATION_FORCE_WAVE64 | (_ZE_PLATFORM_XBOX_SCARLET ? FSR3UPSCALER_SHADER_PERMUTATION_ALLOW_FP16 : 0);
				break;
			case FFX_FSR3UPSCALER_PASS_TCR_AUTOGENERATE:
			default:
				ZE_FAIL("Invalid pass for FSR3!");
				break;
			}
			break;
		}
		case FFX_EFFECT_SSSR:
		{
			switch (passId)
			{
			case FFX_SSSR_PASS_DEPTH_DOWNSAMPLE:
				permutationOptions &= SSSR_SHADER_PERMUTATION_ALLOW_FP16;
				break;
			case FFX_SSSR_PASS_CLASSIFY_TILES:
				permutationOptions &= SSSR_SHADER_PERMUTATION_ALLOW_FP16 | SSSR_SHADER_PERMUTATION_FORCE_WAVE64;
				break;
			case FFX_SSSR_PASS_PREPARE_BLUE_NOISE_TEXTURE:
			case FFX_SSSR_PASS_PREPARE_INDIRECT_ARGS:
			case FFX_SSSR_PASS_INTERSECTION:
				permutationOptions &= SSSR_SHADER_PERMUTATION_FORCE_WAVE64;
				break;
			default:
				ZE_FAIL("Invalid pass for SSSR!");
				break;
			}
			break;
		}
		default:
			ZE_FAIL("Selected effect has not been implemented yet!");
		}
		return static_cast<U64>(effect) | (static_cast<U64>(passId) << 8) | (static_cast<U64>(permutationOptions) << 16);
	}

	FfxErrorCode GetShaderInfo(Device* dev, FfxEffect effect, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader)
	{
		FfxErrorCode code = FFX_OK;
		switch (effect)
		{
		default:
			ZE_FAIL("Selected effect has not been implemented yet!");
			return FFX_ERROR_INVALID_ENUM;
		case FFX_EFFECT_CACAO:
			code = GetShaderInfoCACAO(dev, pass, permutationOptions, shaderBlob, shader);
			break;
		case FFX_EFFECT_DENOISER:
			code = GetShaderInfoDenoiser(dev, pass, permutationOptions, shaderBlob, shader);
			break;
		case FFX_EFFECT_FSR1:
			code = GetShaderInfoFSR1(dev, pass, permutationOptions, shaderBlob, shader);
			break;
		case FFX_EFFECT_FSR2:
			code = GetShaderInfoFSR2(dev, pass, permutationOptions, shaderBlob, shader);
			break;
		case FFX_EFFECT_FSR3UPSCALER:
			code = GetShaderInfoFSR3(dev, pass, permutationOptions, shaderBlob, shader);
			break;
		case FFX_EFFECT_SSSR:
			code = GetShaderInfoSSSR(dev, pass, permutationOptions, shaderBlob, shader);
			break;
		}
		return code;
	}

	FfxErrorCode GetShaderInfoCACAO(Device* dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader)
	{
		static const char* cbvNames[] = { "SSAOConstantsBuffer_t" };
		static const char* samplerNames[] = { "g_PointClampSampler", "g_PointMirrorSampler", "g_LinearClampSampler", "g_ViewspaceDepthTapSampler" };
		static const U32 slots[] = { 0, 1, 2, 3, 4 };
		static const U32 counts[] = { 1, 1, 1, 1, 1 };
		static constexpr const char** POINT_CLAMP_SAMPLER_NAME = samplerNames;
		static constexpr const char** POINT_MIRROR_SAMPLER_NAME = samplerNames + 1;
		static constexpr const char** LINEAR_CLAMP_SAMPLER_NAME = samplerNames + 2;
		static constexpr const char** GENERATE_SAMPLER_NAMES = samplerNames + 1;
		static constexpr const U32* POINT_CLAMP_SAMPLER_SLOT = slots;
		static constexpr const U32* POINT_MIRROR_SAMPLER_SLOT = slots + 1;
		static constexpr const U32* LINEAR_CLAMP_SAMPLER_SLOT = slots + 2;
		static constexpr const U32* GENERATE_SAMPLER_SLOTS = slots + 1;

		const bool fp16 = permutationOptions & CACAO_SHADER_PERMUTATION_ALLOW_FP16;
		const bool wave64 = permutationOptions & CACAO_SHADER_PERMUTATION_FORCE_WAVE64;
		const bool upscaleSmartApply = permutationOptions & CACAO_SHADER_PERMUTATION_APPLY_SMART;
		const bool prepareDownsampled = pass == FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_DEPTHS
			|| pass == FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_DEPTHS_HALF
			|| pass == FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_DEPTHS_AND_MIPS
			|| pass == FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_NORMALS
			|| pass == FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_NORMALS_FROM_INPUT_NORMALS;

		auto getPreparePermutation = [](bool normalsInput, bool half, bool downsampled, bool wave64) -> std::string
			{
				std::string suffix = "";
				if (normalsInput || half || downsampled || wave64)
				{
					suffix = "_";
					if (normalsInput)
						suffix += "I";
					if (half)
						suffix += "F";
					if (downsampled)
						suffix += "D";
					if (wave64)
						suffix += "W";
				}
				return suffix;
			};

		bool applyNonSmart = false;
		bool half = false;
		bool inputNormals = false;
		switch (pass)
		{
		case FFX_CACAO_PASS_CLEAR_LOAD_COUNTER:
		{
			static const char* uavNames[] = { "g_RwLoadCounter" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				0, 0, 1, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				nullptr, nullptr, nullptr, nullptr, // CBV
				nullptr, nullptr, nullptr, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));
			if (shader)
				shader->Init(*dev, "CACAOClearCounterCS");
			break;
		}
		case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_DEPTHS_HALF:
		case FFX_CACAO_PASS_PREPARE_NATIVE_DEPTHS_HALF:
			half = true;
			[[fallthrough]];
		case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_DEPTHS:
		case FFX_CACAO_PASS_PREPARE_NATIVE_DEPTHS:
		{
			static const char* srvNames[] = { "g_DepthIn" };
			static const char* uavNames[] = { "g_RwDeinterleavedDepth" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				POINT_CLAMP_SAMPLER_NAME, POINT_CLAMP_SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "CACAOPrepareDepthCS" + getPreparePermutation(false, half, prepareDownsampled, wave64));
			break;
		}
		case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_DEPTHS_AND_MIPS:
		case FFX_CACAO_PASS_PREPARE_NATIVE_DEPTHS_AND_MIPS:
		{
			static const char* srvNames[] = { "g_DepthIn" };
			static const char* uavNames[] = { "g_RwDepthMips" };
			static const U32 uavCounts[] = { 4 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, uavCounts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				POINT_CLAMP_SAMPLER_NAME, POINT_CLAMP_SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "CACAOPrepareDepthMipsCS" + getPreparePermutation(false, false, prepareDownsampled, wave64));
			break;
		}
		case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_NORMALS_FROM_INPUT_NORMALS:
		case FFX_CACAO_PASS_PREPARE_NATIVE_NORMALS_FROM_INPUT_NORMALS:
			inputNormals = true;
			[[fallthrough]];
		case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_NORMALS:
		case FFX_CACAO_PASS_PREPARE_NATIVE_NORMALS:
		{
			static const char* srvNames[] = { "g_DepthIn" };
			static const char* srvNamesInputNormals[] = { "g_NormalIn" };
			static const char* uavNames[] = { "g_RwDeinterleavedNormals" };
			const FfxShaderBlob blob =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				inputNormals ? srvNamesInputNormals : srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				POINT_CLAMP_SAMPLER_NAME, POINT_CLAMP_SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &blob, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "CACAOPrepareNormalsCS" + getPreparePermutation(inputNormals, false, prepareDownsampled, wave64));
			break;
		}
		case FFX_CACAO_PASS_GENERATE_Q0:
		case FFX_CACAO_PASS_GENERATE_Q1:
		case FFX_CACAO_PASS_GENERATE_Q2:
		case FFX_CACAO_PASS_GENERATE_Q3:
		case FFX_CACAO_PASS_GENERATE_Q3_BASE:
		{
			static const char* srvNames[] = { "g_DeinterleavedDepth", "g_DeinterleavedNormals", "g_LoadCounter", "g_SsaoBufferPong", "g_ImportanceMap" };
			static const char* uavNames[] = { "g_RwSsaoBufferPing" };

			const U8 generateVersion = Utils::SafeCast<U8>(pass - FFX_CACAO_PASS_GENERATE_Q0);
			const FfxShaderBlob blob =
			{
				nullptr, 0, // Blob, data
				1, generateVersion == 3 ? 5U : 2U, 1, 0, 0, 3, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				GENERATE_SAMPLER_NAMES, GENERATE_SAMPLER_SLOTS, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &blob, sizeof(FfxShaderBlob));

			if (shader)
			{
				std::string name = "CACAOGenerateCS";
				if (generateVersion > 0 || wave64)
				{
					name += "_";
					if (generateVersion > 0)
					{
						name += "Q";
						if (generateVersion == 4)
							name += "3B";
						else
							name += std::to_string(generateVersion);
					}
					if (wave64)
						name += "W";
				}
				shader->Init(*dev, name);
			}
			break;
		}
		case FFX_CACAO_PASS_GENERATE_IMPORTANCE_MAP:
		{
			static const char* srvNames[] = { "g_SsaoBufferPong" };
			static const char* uavNames[] = { "g_RwImportanceMap" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				POINT_CLAMP_SAMPLER_NAME, POINT_CLAMP_SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "CACAOImportanceMapGenerateCS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_CACAO_PASS_POST_PROCESS_IMPORTANCE_MAP_A:
		{
			static const char* srvNames[] = { "g_ImportanceMap" };
			static const char* uavNames[] = { "g_RwImportanceMapPong" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				LINEAR_CLAMP_SAMPLER_NAME, LINEAR_CLAMP_SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "CACAOImportanceMapProcessACS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_CACAO_PASS_POST_PROCESS_IMPORTANCE_MAP_B:
		{
			static const char* srvNames[] = { "g_ImportanceMapPong" };
			static const char* uavNames[] = { "g_RwLoadCounter", "g_RwImportanceMap" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 2, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				LINEAR_CLAMP_SAMPLER_NAME, LINEAR_CLAMP_SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "CACAOImportanceMapProcessBCS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_1:
		case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_2:
		case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_3:
		case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_4:
		case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_5:
		case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_6:
		case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_7:
		case FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_8:
		{
			static const char* srvNames[] = { "g_SsaoBufferPing" };
			static const char* uavNames[] = { "g_RwSsaoBufferPong" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				POINT_MIRROR_SAMPLER_NAME, POINT_MIRROR_SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
			{
				std::string name = "CACAOBlurCS";
				const U8 blurCount = Utils::SafeCast<U8>(pass - FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_1 + 1);
				if (blurCount > 1 || fp16 || wave64)
				{
					name += "_";
					if (blurCount > 1)
						name += std::to_string(blurCount);
					if (fp16)
						name += "H";
					if (wave64)
						name += "W";
				}
				shader->Init(*dev, name);
			}
			break;
		}
		case FFX_CACAO_PASS_APPLY_NON_SMART_HALF:
			half = true;
			[[fallthrough]];
		case FFX_CACAO_PASS_APPLY_NON_SMART:
			applyNonSmart = true;
			[[fallthrough]];
		case FFX_CACAO_PASS_APPLY:
		{
			static const char* srvNames[] = { "g_SsaoBufferPing" };
			static const char* uavNames[] = { "g_RwOutput" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				LINEAR_CLAMP_SAMPLER_NAME, LINEAR_CLAMP_SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
			{
				std::string name = "CACAOApplyCS";
				if (applyNonSmart || wave64)
				{
					name += "_";
					if (applyNonSmart)
					{
						name += "NS";
						if (half)
							name += "F";
					}
					if (wave64)
						name += "W";
				}
				shader->Init(*dev, name);
			}
			break;
		}
		case FFX_CACAO_PASS_UPSCALE_BILATERAL_5X5:
		{
			static const char* srvNames[] = { "g_DepthIn", "g_DeinterleavedDepth", "g_SsaoBufferPing" };
			static const char* uavNames[] = { "g_RwOutput" };
			static const char* splrNames[] = { "g_PointClampSampler", "g_LinearClampSampler" };
			static const U32 splrSlots[] = { 0, 2 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 3, 1, 0, 0, 2, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				splrNames, splrSlots, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
			{
				std::string name = "CACAOUpscaleCS";
				if (upscaleSmartApply || fp16 || wave64)
				{
					name += "_";
					if (upscaleSmartApply)
						name += "S";
					if (fp16)
						name += "H";
					if (wave64)
						name += "W";
				}
				shader->Init(*dev, name);
			}
			break;
		}
		default:
			ZE_FAIL("Invalid pass for CACAO!");
			return FFX_ERROR_INVALID_ENUM;
		}
		return FFX_OK;
	}

	FfxErrorCode GetShaderInfoDenoiser(Device* dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader)
	{
		static const char* cbvNamesReflections[] = { "cbDenoiserReflections" };
		static const char* shadowsSamplerNames[] = { "s_trilinerClamp" };
		static const char* reflectionsSamplerNames[] = { "s_LinearSampler" };
		static const U32 slots[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		static const U32 counts[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
		static constexpr const U32* SAMPLER_SLOT = slots;

		const bool fp16 = permutationOptions & DENOISER_SHADER_PERMUTATION_ALLOW_FP16;
		const bool wave64 = permutationOptions & DENOISER_SHADER_PERMUTATION_FORCE_WAVE64;

		switch (pass)
		{
		case FFX_DENOISER_PASS_PREPARE_SHADOW_MASK:
		{
			static const char* cbvName0[] = { "cb0DenoiserShadows" };
			static const char* srvNames[] = { "r_hit_mask_results" };
			static const char* uavNames[] = { "rw_shadow_mask" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 0, 0, 1, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvName0, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				nullptr, nullptr, nullptr, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				uavNames, slots, counts, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "DenoiserShadowsPrepareMaskCS" + GetGeneralPermutation(fp16, wave64));
			break;
		}
		case FFX_DENOISER_PASS_SHADOWS_TILE_CLASSIFICATION:
		{
			static const char* cbvName1[] = { "cb1DenoiserShadows" };
			static const char* srvNames[] = { "r_normal", "r_previous_depth", "r_velocity", "r_depth", "r_previous_moments", "r_history" };
			static const char* uavTexNames[] = { "rw_reprojection_results", "rw_current_moments" };
			static const char* uavBuffNames[] = { "rw_raytracer_result", "rw_tile_metadata" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 6, 2, 0, 2, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvName1, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavTexNames, slots + 2, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				uavBuffNames, slots, counts, nullptr, // UAV buff
				shadowsSamplerNames, SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "DenoiserShadowsClassificationCS" + GetGeneralPermutation(fp16, wave64));
			break;
		}
		case FFX_DENOISER_PASS_FILTER_SOFT_SHADOWS_0:
		case FFX_DENOISER_PASS_FILTER_SOFT_SHADOWS_1:
		case FFX_DENOISER_PASS_FILTER_SOFT_SHADOWS_2:
		{
			static const char* srvNames[] = { "r_depth", "r_normal", "r_filter_input" };
			static const char* uavTexNames[] = { "rw_history" };
			static const char* uavTexNamesLastPass[] = { "rw_filter_output" };
			static const char* uavBuffNames[] = { "rw_tile_metadata" };
			const FfxShaderBlob blob =
			{
				nullptr, 0, // Blob, data
				1, 2U + fp16, 1, 0, 1, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNamesReflections, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				pass == FFX_DENOISER_PASS_FILTER_SOFT_SHADOWS_2 ? uavTexNamesLastPass : uavTexNames, slots + 1, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				uavBuffNames, slots, counts, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &blob, sizeof(FfxShaderBlob));

			if (shader)
			{
				auto getPermutation = [](U8 filterPass, bool fp16, bool wave64) -> std::string
					{
						std::string suffix = "";
						if (filterPass || fp16 || wave64)
						{
							suffix = "_";
							if (filterPass)
								suffix += filterPass == 1 ? "1" : "2";
							if (fp16)
								suffix += "H";
							if (wave64)
								suffix += "W";
						}
						return suffix;
					};
				shader->Init(*dev, "DenoiserShadowsFilterCS" + getPermutation(Utils::SafeCast<U8>(pass) - FFX_DENOISER_PASS_FILTER_SOFT_SHADOWS_0, fp16, wave64));
			}
			break;
		}
		case FFX_DENOISER_PASS_REPROJECT_REFLECTIONS:
		{
			static const char* srvNames[] = { "r_extracted_roughness", "r_radiance", "r_variance", "r_normal", "r_normal_history", "r_radiance_history", "r_roughness_history", "r_depth_history", "r_input_depth_hierarchy", "r_input_motion_vectors", "r_sample_count" };
			static const char* uavTexNames[] = { "rw_reprojected_radiance", "rw_radiance", "rw_sample_count", "rw_average_radiance" };
			static const char* uavBuffNames[] = { "rw_denoiser_tile_list" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 11, 4, 0, 1, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNamesReflections, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavTexNames, slots + 1, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				uavBuffNames, slots, counts, nullptr, // UAV buff
				reflectionsSamplerNames, SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "DenoiserReflectionsReprojectCS" + GetGeneralPermutation(fp16, wave64));
			break;
		}
		case FFX_DENOISER_PASS_PREFILTER_REFLECTIONS:
		{
			static const char* srvNames[] = { "r_radiance", "r_variance", "r_input_normal", "r_extracted_roughness", "r_average_radiance", "r_input_depth_hierarchy" };
			static const char* uavTexNames[] = { "rw_radiance", "rw_variance" };
			static const char* uavBuffNames[] = { "rw_denoiser_tile_list" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 6, 2, 0, 1, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNamesReflections, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavTexNames, slots + 1, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				uavBuffNames, slots, counts, nullptr, // UAV buff
				reflectionsSamplerNames, SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "DenoiserReflectionsPrefilterCS" + GetGeneralPermutation(fp16, wave64));
			break;
		}
		case FFX_DENOISER_PASS_RESOLVE_TEMPORAL_REFLECTIONS:
		{
			static const char* srvNames[] = { "r_extracted_roughness", "r_radiance", "r_variance", "r_sample_count", "r_average_radiance", "r_reprojected_radiance" };
			static const char* uavTexNames[] = { "rw_radiance", "rw_variance" };
			static const char* uavBuffNames[] = { "rw_denoiser_tile_list" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 6, 2, 0, 1, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNamesReflections, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavTexNames, slots + 1, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				uavBuffNames, slots, counts, nullptr, // UAV buff
				reflectionsSamplerNames, SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "DenoiserReflectionsResolveCS" + GetGeneralPermutation(fp16, wave64));
			break;
		}
		default:
			ZE_FAIL("Invalid pass for Denoiser!");
			return FFX_ERROR_INVALID_ENUM;
		}
		return FFX_OK;
	}

	FfxErrorCode GetShaderInfoFSR1(Device* dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader)
	{
		static const char* cbvNames[] = { "cbFSR1" };
		static const char* samplerNames[] = { "s_LinearClamp" };
		static const U32 slots[] = { 0, 1 };
		static const U32 counts[] = { 1 };
		static constexpr const U32* SAMPLER_SLOT = slots;

		const bool passAlpha = permutationOptions & FSR1_SHADER_PERMUTATION_RCAS_PASSTHROUGH_ALPHA;
		const bool srgbConversion = permutationOptions & FSR1_SHADER_PERMUTATION_SRGB_CONVERSIONS;
		const bool fp16 = permutationOptions & FSR1_SHADER_PERMUTATION_ALLOW_FP16;
		const bool wave64 = permutationOptions & FSR1_SHADER_PERMUTATION_FORCE_WAVE64;

		auto getPermutation = [](bool passAlpha, bool sharpen, bool srgbConversion, bool fp16, bool wave64) -> std::string
			{
				std::string suffix = "";
				if (passAlpha || sharpen || srgbConversion || fp16 || wave64)
				{
					suffix = "_";
					if (passAlpha)
						suffix += "A";
					if (sharpen)
						suffix += "S";
					if (srgbConversion)
						suffix += "R";
					if (fp16)
						suffix += "H";
					if (wave64)
						suffix += "W";
				}
				return suffix;
			};

		bool sharpen = false;
		switch (pass)
		{
		case FFX_FSR1_PASS_EASU_RCAS:
			sharpen = true;
			[[fallthrough]];
		case FFX_FSR1_PASS_EASU:
		{
			static const char* srvNames[] = { "r_input_color" };
			static const char* uavNames[] = { "rw_upscaled_output" };
			static const char* uavNamesSharpen[] = { "rw_internal_upscaled_color" };
			const FfxShaderBlob blob =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				sharpen ? uavNamesSharpen : uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				samplerNames, SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &blob, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR1EasuCS" + getPermutation(passAlpha, sharpen, srgbConversion, fp16, wave64));
			break;
		}
		case FFX_FSR1_PASS_RCAS:
		{
			static const char* srvNames[] = { "r_internal_upscaled_color" };
			static const char* uavNames[] = { "rw_upscaled_output" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR1RCasCS" + getPermutation(passAlpha, false, false, fp16, wave64));
			break;
		}
		default:
		{
			ZE_FAIL("Invalid pass for FSR1!");
			return FFX_ERROR_INVALID_ENUM;
		}
		}
		return FFX_OK;
	}

	FfxErrorCode GetShaderInfoFSR2(Device* dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader)
	{
		static const char* samplerNames[] = { "s_LinearClamp" };
		static const char* cbvNamesReactive[] = { "cbFSR2", "cbGenerateReactive" };
		static const U32 slots[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		static const U32 counts[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
		static constexpr const U32* SAMPLER_SLOT = slots;

		const bool depthInverted = permutationOptions & FSR2_SHADER_PERMUTATION_DEPTH_INVERTED;
		const bool lut = permutationOptions & FSR2_SHADER_PERMUTATION_USE_LANCZOS_TYPE;
		const bool lowResMotionVectors = permutationOptions & FSR2_SHADER_PERMUTATION_LOW_RES_MOTION_VECTORS;
		const bool jitteredMotionVectors = permutationOptions & FSR2_SHADER_PERMUTATION_JITTER_MOTION_VECTORS;
		const bool hdr = permutationOptions & FSR2_SHADER_PERMUTATION_HDR_COLOR_INPUT;
		const bool fp16 = permutationOptions & FSR2_SHADER_PERMUTATION_ALLOW_FP16;
		const bool wave64 = permutationOptions & FSR2_SHADER_PERMUTATION_FORCE_WAVE64;

		auto getPermutation = [](bool depthInverted, bool sharpen, bool lut, bool lowResMotionVectors,
			bool jitteredMotionVectors, bool hdr, bool fp16, bool wave64) -> std::string
			{
				std::string suffix = "";
				if (depthInverted || sharpen || lut || lowResMotionVectors || jitteredMotionVectors || hdr || fp16 || wave64)
				{
					suffix = "_";
					if (depthInverted)
						suffix += "I";
					if (sharpen)
						suffix += "S";
					if (lut)
						suffix += "L";
					if (lowResMotionVectors)
						suffix += "R";
					if (jitteredMotionVectors)
						suffix += "J";
					if (hdr)
						suffix += "D";
					if (fp16)
						suffix += "H";
					if (wave64)
						suffix += "W";
				}
				return suffix;
			};

		bool sharpen = false;
		switch (pass)
		{
		case FFX_FSR2_PASS_DEPTH_CLIP:
		{
			static const char* srvNames[] = { "r_input_color_jittered", "r_input_motion_vectors", "r_input_exposure", "r_reactive_mask", "r_transparency_and_composition_mask", "r_reconstructed_previous_nearest_depth", "r_dilated_motion_vectors", "r_previous_dilated_motion_vectors", "r_dilatedDepth", "r_input_depth" };
			static const char* uavNames[] = { "rw_prepared_input_color", "rw_dilated_reactive_masks" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 10, 2, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNamesReactive, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				samplerNames, SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR2DepthClipCS" + getPermutation(depthInverted, false, false, lowResMotionVectors, jitteredMotionVectors, false, fp16, wave64));
			break;
		}
		case FFX_FSR2_PASS_RECONSTRUCT_PREVIOUS_DEPTH:
		{
			static const char* srvNames[] = { "r_input_color_jittered", "r_input_motion_vectors", "r_input_depth", "r_input_exposure" };
			static const char* uavNames[] = { "rw_reconstructed_previous_nearest_depth", "rw_dilated_motion_vectors", "rw_dilatedDepth", "rw_lock_input_luma" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 4, 4, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNamesReactive, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR2ReconstructPrevDepthCS" + getPermutation(depthInverted, false, false, lowResMotionVectors, jitteredMotionVectors, hdr, fp16, wave64));
			break;
		}
		case FFX_FSR2_PASS_LOCK:
		{
			static const char* srvNames[] = { "r_lock_input_luma" };
			static const char* uavNames[] = { "rw_reconstructed_previous_nearest_depth", "rw_new_locks" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 2, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNamesReactive, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR2LockCS" + getPermutation(depthInverted, false, false, false, false, false, fp16, wave64));
			break;
		}
		case FFX_FSR2_PASS_ACCUMULATE_SHARPEN:
			sharpen = true;
			[[fallthrough]];
		case FFX_FSR2_PASS_ACCUMULATE:
		{
			static const char* srvNames[] = { "r_input_exposure", "r_input_motion_vectors", "r_internal_upscaled_color", "r_lock_status", "r_prepared_input_color", "r_luma_history", "r_imgMips", "r_dilated_reactive_masks" };
			static const char* srvNamesLut[] = { "r_input_exposure", "r_input_motion_vectors", "r_internal_upscaled_color", "r_lock_status", "r_prepared_input_color", "r_luma_history", "r_imgMips", "r_dilated_reactive_masks", "r_lanczos_lut" };
			static const char* srvNamesLowRes[] = { "r_input_exposure", "r_dilated_motion_vectors", "r_internal_upscaled_color", "r_lock_status", "r_prepared_input_color", "r_luma_history", "r_imgMips", "r_dilated_reactive_masks" };
			static const char* srvNamesLowResLut[] = { "r_input_exposure", "r_dilated_motion_vectors", "r_internal_upscaled_color", "r_lock_status", "r_prepared_input_color", "r_luma_history", "r_imgMips", "r_dilated_reactive_masks", "r_lanczos_lut" };
			static const char* uavNames[] = { "rw_internal_upscaled_color", "rw_lock_status", "rw_new_locks", "rw_luma_history", "rw_upscaled_output" };
			const FfxShaderBlob blob =
			{
				nullptr, 0, // Blob, data
				1, 8 + static_cast<U32>(lut), 4 + static_cast<U32>(!sharpen), 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNamesReactive, slots, counts, nullptr, // CBV
				lut ? (lowResMotionVectors ? srvNamesLowResLut : srvNamesLut) : (lowResMotionVectors ? srvNamesLowRes : srvNames), slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				samplerNames, SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &blob, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR2AccumulateCS" + getPermutation(false, sharpen, lut, lowResMotionVectors, jitteredMotionVectors, hdr, fp16, wave64));
			break;
		}
		case FFX_FSR2_PASS_RCAS:
		{
			static const char* cbvNames[] = { "cbFSR2", "cbRCAS" };
			static const char* srvNames[] = { "r_input_exposure", "r_rcas_input" };
			static const char* uavNames[] = { "rw_upscaled_output" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				2, 2, 1, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR2RCasCS" + GetGeneralPermutation(_ZE_PLATFORM_XBOX_SCARLET ? fp16 : false, wave64));
			break;
		}
		case FFX_FSR2_PASS_COMPUTE_LUMINANCE_PYRAMID:
		{
			static const char* cbvNames[] = { "cbFSR2", "cbSPD" };
			static const char* srvNames[] = { "r_input_color_jittered" };
			static const char* uavNames[] = { "rw_img_mip_shading_change", "rw_img_mip_5", "rw_auto_exposure", "rw_spd_global_atomic" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				2, 1, 4, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				samplerNames, SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR2LuminancePyramidCS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_FSR2_PASS_GENERATE_REACTIVE:
		{
			static const char* srvNames[] = { "r_input_color_jittered", "r_input_opaque_only" };
			static const char* uavNames[] = { "rw_output_autoreactive" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				2, 2, 1, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNamesReactive, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR2GenerateReactiveCS" + GetGeneralPermutation(fp16, wave64));
			break;
		}
		case FFX_FSR2_PASS_TCR_AUTOGENERATE:
		{
			static const char* srvNames[] = { "r_input_color_jittered", "r_input_opaque_only", "r_input_motion_vectors", "r_reactive_mask", "r_transparency_and_composition_mask", "r_input_prev_color_pre_alpha", "r_input_prev_color_post_alpha" };
			static const char* uavNames[] = { "rw_output_autoreactive", "rw_output_autocomposition", "rw_output_prev_color_pre_alpha", "rw_output_prev_color_post_alpha" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				2, 7, 4, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNamesReactive, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR2GenerateTCRCS" + getPermutation(false, false, false, false, jitteredMotionVectors, false, fp16, wave64));
			break;
		}
		default:
			ZE_FAIL("Invalid pass for FSR2!");
			return FFX_ERROR_INVALID_ENUM;
		}
		return FFX_OK;
	}

	FfxErrorCode GetShaderInfoFSR3(Device* dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader)
	{
		static const char* samplerNames[] = { "s_LinearClamp" };
		static const U32 slots[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		static const U32 counts[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
		static constexpr const U32* SAMPLER_SLOT = slots;

		const bool depthInverted = permutationOptions & FSR3UPSCALER_SHADER_PERMUTATION_DEPTH_INVERTED;
		const bool lut = permutationOptions & FSR3UPSCALER_SHADER_PERMUTATION_USE_LANCZOS_TYPE;
		const bool lowResMotionVectors = permutationOptions & FSR3UPSCALER_SHADER_PERMUTATION_LOW_RES_MOTION_VECTORS;
		const bool jitteredMotionVectors = permutationOptions & FSR3UPSCALER_SHADER_PERMUTATION_JITTER_MOTION_VECTORS;
		const bool hdr = permutationOptions & FSR3UPSCALER_SHADER_PERMUTATION_HDR_COLOR_INPUT;
		const bool fp16 = permutationOptions & FSR3UPSCALER_SHADER_PERMUTATION_ALLOW_FP16;
		const bool wave64 = permutationOptions & FSR3UPSCALER_SHADER_PERMUTATION_FORCE_WAVE64;

		auto getPermutation = [](bool depthInverted, bool sharpen, bool lut, bool lowResMotionVectors,
			bool jitteredMotionVectors, bool hdr, bool fp16, bool wave64) -> std::string
			{
				std::string suffix = "";
				if (depthInverted || sharpen || lut || lowResMotionVectors || jitteredMotionVectors || hdr || fp16 || wave64)
				{
					suffix = "_";
					if (depthInverted)
						suffix += "I";
					if (sharpen)
						suffix += "S";
					if (lut)
						suffix += "L";
					if (lowResMotionVectors)
						suffix += "R";
					if (jitteredMotionVectors)
						suffix += "J";
					if (hdr)
						suffix += "D";
					if (fp16)
						suffix += "H";
					if (wave64)
						suffix += "W";
				}
				return suffix;
			};

		bool sharpen = false;
		switch (pass)
		{
		case FFX_FSR3UPSCALER_PASS_PREPARE_INPUTS:
		{
			static const char* cbvNames[] = { "cbFSR3Upscaler" };
			static const char* srvNames[] = { "r_input_depth", "r_input_color_jittered", "r_input_motion_vectors" };
			static const char* uavNames[] = { "rw_farthest_depth", "rw_current_luma", "rw_reconstructed_previous_nearest_depth", "rw_dilated_depth", "rw_dilated_motion_vectors" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 3, 5, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR3PrepareInputsCS" + getPermutation(depthInverted, false, false, lowResMotionVectors, jitteredMotionVectors, false, false, wave64));
			break;
		}
		case FFX_FSR3UPSCALER_PASS_LUMA_PYRAMID:
		{
			static const char* cbvNames[] = { "cbFSR3Upscaler", "cbSPD" };
			static const char* srvNames[] = { "r_farthest_depth", "r_current_luma" };
			static const char* uavNames[] = { "rw_farthest_depth_mip1", "rw_frame_info", "rw_spd_mip0", "rw_spd_mip1", "rw_spd_mip2", "rw_spd_mip3", "rw_spd_mip4", "rw_spd_mip5", "rw_spd_global_atomic" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				2, 2, 9, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR3LumaPyramidCS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_FSR3UPSCALER_PASS_SHADING_CHANGE_PYRAMID:
		{
			static const char* cbvNames[] = { "cbFSR3Upscaler", "cbSPD" };
			static const char* srvNames[] = { "r_current_luma", "r_previous_luma", "r_dilated_motion_vectors", "r_input_exposure" };
			static const char* uavNames[] = { "rw_spd_mip0", "rw_spd_mip1", "rw_spd_mip2", "rw_spd_mip3", "rw_spd_mip4", "rw_spd_mip5", "rw_spd_global_atomic" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				2, 4, 7, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR3ShadingChangePyramidCS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_FSR3UPSCALER_PASS_SHADING_CHANGE:
		{
			static const char* cbvNames[] = { "cbFSR3Upscaler" };
			static const char* srvNames[] = { "r_spd_mips" };
			static const char* uavNames[] = { "rw_shading_change" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				samplerNames, SAMPLER_SLOT, SAMPLER_SLOT, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR3ShadingChangeCS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_FSR3UPSCALER_PASS_PREPARE_REACTIVITY:
		{
			static const char* cbvNames[] = { "cbFSR3Upscaler" };
			static const char* srvNames[] = { "r_reactive_mask", "r_transparency_and_composition_mask", "r_accumulation", "r_shading_change", "r_current_luma", "r_reconstructed_previous_nearest_depth", "r_dilated_motion_vectors", "r_dilated_depth", "r_input_exposure" };
			static const char* uavNames[] = { "rw_accumulation", "rw_new_locks", "rw_dilated_reactive_masks" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 9, 3, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				samplerNames, SAMPLER_SLOT, SAMPLER_SLOT, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR3PrepareReactivityCS" + GetGeneralPermutation(fp16, wave64));
			break;
		}
		case FFX_FSR3UPSCALER_PASS_LUMA_INSTABILITY:
		{
			static const char* cbvNames[] = { "cbFSR3Upscaler" };
			static const char* srvNames[] = { "r_luma_history", "r_current_luma", "r_dilated_motion_vectors", "r_input_exposure", "r_dilated_reactive_masks", "r_farthest_depth_mip1" };
			static const char* uavNames[] = { "rw_luma_history", "rw_luma_instability" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 6, 2, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				samplerNames, SAMPLER_SLOT, SAMPLER_SLOT, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR3LumaInstabilityCS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_FSR3UPSCALER_PASS_ACCUMULATE_SHARPEN:
			sharpen = true;
			[[fallthrough]];
		case FFX_FSR3UPSCALER_PASS_ACCUMULATE:
		{
			static const char* cbvNames[] = { "cbFSR3Upscaler" };
			static const char* srvNames[] = { "r_input_color_jittered", "r_input_motion_vectors", "r_internal_upscaled_color", "r_farthest_depth_mip1", "r_luma_instability", "r_input_exposure", "r_dilated_reactive_masks" };
			static const char* srvNamesLut[] = { "r_input_color_jittered", "r_input_motion_vectors", "r_internal_upscaled_color", "r_farthest_depth_mip1", "r_luma_instability", "r_input_exposure", "r_lanczos_lut", "r_dilated_reactive_masks" };
			static const char* srvNamesLowRes[] = { "r_input_color_jittered", "r_internal_upscaled_color", "r_farthest_depth_mip1", "r_luma_instability", "r_dilated_motion_vectors", "r_input_exposure", "r_dilated_reactive_masks" };
			static const char* srvNamesLowResLut[] = { "r_input_color_jittered", "r_internal_upscaled_color", "r_farthest_depth_mip1", "r_luma_instability", "r_dilated_motion_vectors", "r_input_exposure", "r_lanczos_lut", "r_dilated_reactive_masks" };
			static const char* uavNames[] = { "rw_internal_upscaled_color", "rw_upscaled_output", "rw_new_locks" };
			static const char* uavNamesSharpen[] = { "rw_internal_upscaled_color", "rw_new_locks" };
			const FfxShaderBlob blob =
			{
				nullptr, 0, // Blob, data
				1, lut ? 8U : 7U, 2U + static_cast<U32>(sharpen), 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				lowResMotionVectors ? (lut ? srvNamesLowResLut : srvNamesLowRes) : (lut ? srvNamesLut : srvNames), slots, counts, nullptr, // SRV tex
				sharpen ? uavNamesSharpen : uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				samplerNames, SAMPLER_SLOT, SAMPLER_SLOT, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &blob, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR3AccumulateCS" + getPermutation(false, sharpen, lut, lowResMotionVectors, jitteredMotionVectors, hdr, fp16, wave64));
			break;
		}
		case FFX_FSR3UPSCALER_PASS_RCAS:
		{
			static const char* cbvNames[] = { "cbRCAS" };
			static const char* srvNames[] = { "r_rcas_input", "r_input_exposure" };
			static const char* uavNames[] = { "rw_upscaled_output" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 2, 1, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR3RCasCS" + GetGeneralPermutation(_ZE_PLATFORM_XBOX_SCARLET ? fp16 : false, wave64));
			break;
		}
		case FFX_FSR3UPSCALER_PASS_DEBUG_VIEW:
		{
			static const char* cbvNames[] = { "cbFSR3Upscaler" };
			static const char* srvNames[] = { "r_internal_upscaled_color", "r_dilated_motion_vectors", "r_dilated_depth", "r_dilated_reactive_masks" };
			static const char* uavNames[] = { "rw_upscaled_output" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 2, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				samplerNames, SAMPLER_SLOT, SAMPLER_SLOT, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR3DebugViewCS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_FSR3UPSCALER_PASS_GENERATE_REACTIVE:
		{
			static const char* cbvNames[] = { "cbFSR3Upscaler", "cbGenerateReactive" };
			static const char* srvNames[] = { "r_input_color_jittered", "r_input_opaque_only" };
			static const char* uavNames[] = { "rw_output_autoreactive" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				2, 2, 1, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "FSR3GenerateReactiveCS" + GetGeneralPermutation(fp16, wave64));
			break;
		}
		case FFX_FSR3UPSCALER_PASS_TCR_AUTOGENERATE:
		default:
			ZE_FAIL("Invalid pass for FSR3!");
			return FFX_ERROR_INVALID_ENUM;
		}
		return FFX_OK;
	}

	FfxErrorCode GetShaderInfoSSSR(Device* dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader)
	{
		static const char* cbvNames[] = { "cbSSSR" };
		static const char* samplerNames[] = { "s_EnvironmentMapSampler" };
		static const U32 slots[] = { 0, 1, 2, 3, 4, 5 };
		static const U32 counts[] = { 1, 1, 1, 1, 1, 1, 13 };
		static constexpr const U32* SAMPLER_SLOT = slots;
		static constexpr const U32* DEPTH_DOWNSAMPLE_UAV_COUNTS = counts + 6;

		const bool fp16 = permutationOptions & SSSR_SHADER_PERMUTATION_ALLOW_FP16;
		const bool wave64 = permutationOptions & SSSR_SHADER_PERMUTATION_FORCE_WAVE64;

		switch (pass)
		{
		case FFX_SSSR_PASS_DEPTH_DOWNSAMPLE:
		{
			static const char* srvNames[] = { "r_input_depth" };
			static const char* uavTexNames[] = { "rw_depth_hierarchy" };
			static const char* uavBuffNames[] = { "rw_spd_global_atomic" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				0, 1, 1, 0, 1, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				nullptr, nullptr, nullptr, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavTexNames, slots + 1, DEPTH_DOWNSAMPLE_UAV_COUNTS, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				uavBuffNames, slots, counts, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "SSSRDepthDownsampleCS" + GetGeneralPermutation(fp16, false));
			break;
		}
		case FFX_SSSR_PASS_CLASSIFY_TILES:
		{
			static const char* srvNames[] = { "r_depth_hierarchy", "r_variance", "r_input_normal", "r_input_environment_map", "r_input_material_parameters" };
			static const char* uavTexNames[] = { "rw_radiance", "rw_extracted_roughness" };
			static const char* uavBuffNames[] = { "rw_ray_counter", "rw_ray_list", "rw_denoiser_tile_list" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 5, 2, 0, 3, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavTexNames, slots + 3, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				uavBuffNames, slots, counts, nullptr, // UAV buff
				samplerNames, SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "SSSRClassifyTilesCS" + GetGeneralPermutation(fp16, wave64));
			break;
		}
		case FFX_SSSR_PASS_PREPARE_BLUE_NOISE_TEXTURE:
		{
			static const char* srvNames[] = { "r_sobol_buffer", "r_scrambling_tile_buffer" };
			static const char* uavNames[] = { "rw_blue_noise_texture" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 2, 1, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavNames, slots, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "SSSRPrepareNoiseCS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_SSSR_PASS_PREPARE_INDIRECT_ARGS:
		{
			static const char* uavNames[] = { "rw_ray_counter", "rw_intersection_pass_indirect_args" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				0, 0, 0, 0, 2, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				nullptr, nullptr, nullptr, nullptr, // CBV
				nullptr, nullptr, nullptr, nullptr, // SRV tex
				nullptr, nullptr, nullptr, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				uavNames, slots, counts, nullptr, // UAV buff
				nullptr, nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "SSSRPrepareIndirectCS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_SSSR_PASS_INTERSECTION:
		{
			static const char* srvNames[] = { "r_blue_noise_texture", "r_depth_hierarchy", "r_input_normal", "r_extracted_roughness", "r_input_color", "r_input_environment_map" };
			static const char* uavTexNames[] = { "rw_radiance" };
			static const char* uavBuffNames[] = { "rw_ray_counter", "rw_ray_list" };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 6, 1, 0, 2, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, slots, counts, nullptr, // CBV
				srvNames, slots, counts, nullptr, // SRV tex
				uavTexNames, slots + 2, counts, nullptr, // UAV tex
				nullptr, nullptr, nullptr, nullptr, // SRV buff
				uavBuffNames, slots, counts, nullptr, // UAV buff
				samplerNames, SAMPLER_SLOT, counts, nullptr, // Samplers
				nullptr, nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(*dev, "SSSRIntersectCS" + GetGeneralPermutation(false, wave64));
			break;
		}
		default:
			ZE_FAIL("Invalid pass for SSSR!");
			return FFX_ERROR_INVALID_ENUM;
		}
		return FFX_OK;
	}

	std::string GetGeneralPermutation(bool fp16, bool wave64) noexcept
	{
		if (fp16)
		{
			if (wave64)
				return "_HW";
			return "_H";
		}
		else if (wave64)
			return "_W";
		return "";
	}
}