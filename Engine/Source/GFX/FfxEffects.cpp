#include "GFX/FfxEffects.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_cacao.h"
#include "FidelityFX/host/ffx_fsr2.h"
#include "../src/components/cacao/ffx_cacao_private.h"
#include "../src/components/fsr2/ffx_fsr2_private.h"
ZE_WARNING_POP

namespace ZE::GFX::FFX
{
	FfxErrorCode GetShaderInfoCACAO(Device& dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader);
	FfxErrorCode GetShaderInfoFSR2(Device& dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader);
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
		case FFX_EFFECT_FSR2:
		{
			switch (passId)
			{
			case FFX_FSR2_PASS_DEPTH_CLIP: // TODO
			case FFX_FSR2_PASS_RECONSTRUCT_PREVIOUS_DEPTH: // TODO
			case FFX_FSR2_PASS_LOCK: // TODO
				break;
			case FFX_FSR2_PASS_ACCUMULATE_SHARPEN:
				permutationOptions |= FSR2_SHADER_PERMUTATION_ENABLE_SHARPENING; // Indicate that sharpening will always happen here
				[[fallthrough]];
			case FFX_FSR2_PASS_ACCUMULATE:
				permutationOptions &= FSR2_SHADER_PERMUTATION_FORCE_WAVE64 | FSR2_SHADER_PERMUTATION_ALLOW_FP16
					| FSR2_SHADER_PERMUTATION_HDR_COLOR_INPUT | FSR2_SHADER_PERMUTATION_LOW_RES_MOTION_VECTORS
					| FSR2_SHADER_PERMUTATION_USE_LANCZOS_TYPE | FSR2_SHADER_PERMUTATION_JITTER_MOTION_VECTORS;
				break;
			case FFX_FSR2_PASS_RCAS:
			case FFX_FSR2_PASS_COMPUTE_LUMINANCE_PYRAMID:
				permutationOptions &= FSR2_SHADER_PERMUTATION_FORCE_WAVE64 | FSR2_SHADER_PERMUTATION_ALLOW_FP16;
				break;
			case FFX_FSR2_PASS_GENERATE_REACTIVE: // TODO
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
		default:
			ZE_FAIL("Selected effect has not been implemented yet!");
		}
		return static_cast<U64>(effect) | (static_cast<U64>(passId) << 8) | (static_cast<U64>(permutationOptions) << 16);
	}

	FfxErrorCode GetShaderInfo(Device& dev, FfxEffect effect, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader)
	{
		FfxErrorCode code = FFX_OK;
		switch (effect)
		{
		default:
			ZE_FAIL("Selected effect has not been implemented yet!");
			return FFX_ERROR_INVALID_ENUM;
		case FFX_EFFECT_FSR2:
			code = GetShaderInfoFSR2(dev, pass, permutationOptions, shaderBlob, shader);
			break;
		case FFX_EFFECT_CACAO:
			code = GetShaderInfoCACAO(dev, pass, permutationOptions, shaderBlob, shader);
			break;
		}
		return code;
	}

	FfxErrorCode GetShaderInfoCACAO(Device& dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader)
	{
		static const char* cbvNames[] = { "SSAOConstantsBuffer_t" };
		static const U32 cbvSlots[] = { 0 };
		static const U32 cbvCounts[] = { 1 };

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
		bool isQ3Base = false;
		switch (pass)
		{
		case FFX_CACAO_PASS_CLEAR_LOAD_COUNTER:
		{
			static const char* uavNames[] = { "g_RwLoadCounter" };
			static const U32 uavSlots[] = { 0 };
			static const U32 uavCounts[] = { 1 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				0, 0, 1, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				nullptr, nullptr, nullptr, // CBV
				nullptr, nullptr, nullptr, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));
			if (shader)
				shader->Init(dev, "CACAOClearCounterCS");
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
			static const U32 srvSlots[] = { 0 };
			static const U32 srvCounts[] = { 1 };
			static const char* uavNames[] = { "g_RwDeinterleavedDepth" };
			static const U32 uavSlots[] = { 0 };
			static const U32 uavCounts[] = { 1 };
			static const char* samplerNames[] = { "g_PointClampSampler" };
			static const U32 samplerSlots[] = { 0 };
			static const U32 samplerCounts[] = { 1 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(dev, "CACAOPrepareDepthCS" + getPreparePermutation(false, half, prepareDownsampled, wave64));
			break;
		}
		case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_DEPTHS_AND_MIPS:
		case FFX_CACAO_PASS_PREPARE_NATIVE_DEPTHS_AND_MIPS:
		{
			static const char* srvNames[] = { "g_DepthIn" };
			static const U32 srvSlots[] = { 0 };
			static const U32 srvCounts[] = { 1 };
			static const char* uavNames[] = { "g_RwDepthMips" };
			static const U32 uavSlots[] = { 0 };
			static const U32 uavCounts[] = { 4 };
			static const char* samplerNames[] = { "g_PointClampSampler" };
			static const U32 samplerSlots[] = { 0 };
			static const U32 samplerCounts[] = { 1 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(dev, "CACAOPrepareDepthMipsCS" + getPreparePermutation(false, false, prepareDownsampled, wave64));
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
			static const U32 srvSlots[] = { 0 };
			static const U32 srvCounts[] = { 1 };
			static const char* uavNames[] = { "g_RwDeinterleavedNormals" };
			static const U32 uavSlots[] = { 0 };
			static const U32 uavCounts[] = { 1 };
			static const char* samplerNames[] = { "g_PointClampSampler" };
			static const U32 samplerSlots[] = { 0 };
			static const U32 samplerCounts[] = { 1 };
			FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				inputNormals ? srvNamesInputNormals : srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(dev, "CACAOPrepareNormalsCS" + getPreparePermutation(inputNormals, false, prepareDownsampled, wave64));
			break;
		}
		case FFX_CACAO_PASS_GENERATE_Q3_BASE:
			isQ3Base = true;
			[[fallthrough]];
		case FFX_CACAO_PASS_GENERATE_Q0:
		case FFX_CACAO_PASS_GENERATE_Q1:
		case FFX_CACAO_PASS_GENERATE_Q2:
		case FFX_CACAO_PASS_GENERATE_Q3:
		{
			static const char* srvNames[] = { "g_DeinterleavedDepth", "g_DeinterleavedNormals", "g_LoadCounter", "g_SsaoBufferPong", "g_ImportanceMap" };
			static const U32 srvSlots[] = { 0, 1, 2, 3, 4 };
			static const U32 srvCounts[] = { 1, 1, 1, 1, 1 };
			static const char* uavNames[] = { "g_RwSsaoBufferPing" };
			static const U32 uavSlots[] = { 0 };
			static const U32 uavCounts[] = { 1 };
			static const char* samplerNames[] = { "g_PointMirrorSampler", "g_LinearClampSampler", "g_ViewspaceDepthTapSampler" };
			static const U32 samplerSlots[] = { 1, 2, 3 };
			static const U32 samplerCounts[] = { 1, 1, 1 };
			FfxShaderBlob blob =
			{
				nullptr, 0, // Blob, data
				1, isQ3Base ? 5U : 2U, 1, 0, 0, 3, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &blob, sizeof(FfxShaderBlob));

			if (shader)
			{
				std::string name = "CACAOGenerateCS";
				const U8 generateVersion = Utils::SafeCast<U8>(pass - FFX_CACAO_PASS_GENERATE_Q0);
				if (generateVersion > 1 || wave64)
				{
					name += "_";
					if (generateVersion > 1)
					{
						name += "Q";
						if (isQ3Base)
							name += "3B";
						else
							name += std::to_string(generateVersion);
					}
					if (wave64)
						name += "W";
				}
				shader->Init(dev, name);
			}
			break;
		}
		case FFX_CACAO_PASS_GENERATE_IMPORTANCE_MAP:
		{
			static const char* srvNames[] = { "g_SsaoBufferPong" };
			static const U32 srvSlots[] = { 0 };
			static const U32 srvCounts[] = { 1 };
			static const char* uavNames[] = { "g_RwImportanceMap" };
			static const U32 uavSlots[] = { 0 };
			static const U32 uavCounts[] = { 1 };
			static const char* samplerNames[] = { "g_PointClampSampler" };
			static const U32 samplerSlots[] = { 0 };
			static const U32 samplerCounts[] = { 1 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(dev, "CACAOImportanceMapGenerateCS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_CACAO_PASS_POST_PROCESS_IMPORTANCE_MAP_A:
		{
			static const char* srvNames[] = { "g_ImportanceMap" };
			static const U32 srvSlots[] = { 0 };
			static const U32 srvCounts[] = { 1 };
			static const char* uavNames[] = { "g_RwImportanceMapPong" };
			static const U32 uavSlots[] = { 0 };
			static const U32 uavCounts[] = { 1 };
			static const char* samplerNames[] = { "g_LinearClampSampler" };
			static const U32 samplerSlots[] = { 2 };
			static const U32 samplerCounts[] = { 1 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(dev, "CACAOImportanceMapProcessACS" + GetGeneralPermutation(false, wave64));
			break;
		}
		case FFX_CACAO_PASS_POST_PROCESS_IMPORTANCE_MAP_B:
		{
			static const char* srvNames[] = { "g_ImportanceMapPong" };
			static const U32 srvSlots[] = { 0 };
			static const U32 srvCounts[] = { 1 };
			static const char* uavNames[] = { "g_RwLoadCounter", "g_RwImportanceMap" };
			static const U32 uavSlots[] = { 0, 1 };
			static const U32 uavCounts[] = { 1, 1 };
			static const char* samplerNames[] = { "g_LinearClampSampler" };
			static const U32 samplerSlots[] = { 2 };
			static const U32 samplerCounts[] = { 1 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 2, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(dev, "CACAOImportanceMapProcessBCS" + GetGeneralPermutation(false, wave64));
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
			static const U32 srvSlots[] = { 0 };
			static const U32 srvCounts[] = { 1 };
			static const char* uavNames[] = { "g_RwSsaoBufferPong" };
			static const U32 uavSlots[] = { 0 };
			static const U32 uavCounts[] = { 1 };
			static const char* samplerNames[] = { "g_PointMirrorSampler" };
			static const U32 samplerSlots[] = { 1 };
			static const U32 samplerCounts[] = { 1 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
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
				shader->Init(dev, name);
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
			static const U32 srvSlots[] = { 0 };
			static const U32 srvCounts[] = { 1 };
			static const char* uavNames[] = { "g_RwOutput" };
			static const U32 uavSlots[] = { 0 };
			static const U32 uavCounts[] = { 1 };
			static const char* samplerNames[] = { "g_LinearClampSampler" };
			static const U32 samplerSlots[] = { 2 };
			static const U32 samplerCounts[] = { 1 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 1, 1, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
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
				shader->Init(dev, name);
			}
			break;
		}
		case FFX_CACAO_PASS_UPSCALE_BILATERAL_5X5:
		{
			static const char* srvNames[] = { "g_DepthIn", "g_DeinterleavedDepth", "g_SsaoBufferPing" };
			static const U32 srvSlots[] = { 0, 1, 2 };
			static const U32 srvCounts[] = { 1, 1, 1 };
			static const char* uavNames[] = { "g_RwOutput" };
			static const U32 uavSlots[] = { 0 };
			static const U32 uavCounts[] = { 1 };
			static const char* samplerNames[] = { "g_PointClampSampler", "g_LinearClampSampler" };
			static const U32 samplerSlots[] = { 0, 2 };
			static const U32 samplerCounts[] = { 1, 1 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 3, 1, 0, 0, 2, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
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
				shader->Init(dev, name);
			}
			break;
		}
		default:
			ZE_FAIL("Invalid pass for CACAO!");
			return FFX_ERROR_INVALID_ENUM;
		}
		return FFX_OK;
	}

	FfxErrorCode GetShaderInfoFSR2(Device& dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader)
	{
		const bool depthInverted = permutationOptions & FSR2_SHADER_PERMUTATION_DEPTH_INVERTED;
		const bool sharpen = permutationOptions & FSR2_SHADER_PERMUTATION_ENABLE_SHARPENING;
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

		switch (pass)
		{
		case FFX_FSR2_PASS_DEPTH_CLIP:
		{
			break;
		}
		case FFX_FSR2_PASS_RECONSTRUCT_PREVIOUS_DEPTH:
		{
			break;
		}
		case FFX_FSR2_PASS_LOCK:
		{
			break;
		}
		case FFX_FSR2_PASS_ACCUMULATE:
		case FFX_FSR2_PASS_ACCUMULATE_SHARPEN:
		{
			static const char* cbvNames[] = { "cbFSR2" };
			static const U32 cbvSlots[] = { 0 };
			static const U32 cbvCounts[] = { 1 };
			static const char* srvNames[] = { "r_input_exposure", "r_dilated_motion_vectors", "r_internal_upscaled_color", "r_lock_status", "r_prepared_input_color", "r_luma_history", "r_imgMips", "r_dilated_reactive_masks" };
			static const char* srvNamesLut[] = { "r_input_exposure", "r_dilated_motion_vectors", "r_internal_upscaled_color", "r_lock_status", "r_prepared_input_color", "r_luma_history", "r_imgMips", "r_dilated_reactive_masks", "r_lanczos_lut" };
			static const char* srvNamesLowRes[] = { "r_input_exposure", "r_input_motion_vectors", "r_internal_upscaled_color", "r_lock_status", "r_prepared_input_color", "r_luma_history", "r_imgMips", "r_dilated_reactive_masks" };
			static const char* srvNamesLowResLut[] = { "r_input_exposure", "r_input_motion_vectors", "r_internal_upscaled_color", "r_lock_status", "r_prepared_input_color", "r_luma_history", "r_imgMips", "r_dilated_reactive_masks", "r_lanczos_lut" };
			static const U32 srvSlots[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
			static const U32 srvCounts[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
			static const char* uavNames[] = { "rw_internal_upscaled_color", "rw_lock_status", "rw_new_locks", "rw_luma_history", "rw_upscaled_output" };
			static const U32 uavSlots[] = { 0, 1, 2, 3, 4 };
			static const U32 uavCounts[] = { 1, 1, 1, 1, 1 };
			static const char* samplerNames[] = { "s_LinearClamp" };
			static const U32 samplerSlots[] = { 2 };
			static const U32 samplerCounts[] = { 1 };
			FfxShaderBlob blob =
			{
				nullptr, 0, // Blob, data
				1, 8 + static_cast<U32>(lut), 4 + static_cast<U32>(sharpen), 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				lut ? (lowResMotionVectors ? srvNamesLowResLut : srvNamesLut) : (lowResMotionVectors ? srvNamesLowRes : srvNames) , srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &blob, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(dev, "FSR2AccumulateCS" + getPermutation(false, sharpen, lut, lowResMotionVectors, jitteredMotionVectors, hdr, fp16, wave64));
			break;
		}
		case FFX_FSR2_PASS_RCAS:
		{
			static const char* cbvNames[] = { "cbFSR2", "cbRCAS" };
			static const U32 cbvSlots[] = { 0, 1 };
			static const U32 cbvCounts[] = { 1, 1 };
			static const char* srvNames[] = { "r_input_exposure", "r_rcas_input" };
			static const U32 srvSlots[] = { 0, 1 };
			static const U32 srvCounts[] = { 1, 1 };
			static const char* uavNames[] = { "rw_upscaled_output" };
			static const U32 uavSlots[] = { 0 };
			static const U32 uavCounts[] = { 1 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				2, 2, 1, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(dev, "FSR2RCasCS" + GetGeneralPermutation(fp16, wave64));
			break;
		}
		case FFX_FSR2_PASS_COMPUTE_LUMINANCE_PYRAMID:
		{
			static const char* cbvNames[] = { "cbFSR2", "cbSPD" };
			static const U32 cbvSlots[] = { 0, 1 };
			static const U32 cbvCounts[] = { 1, 1 };
			static const char* srvNames[] = { "r_input_color_jittered" };
			static const U32 srvSlots[] = { 0 };
			static const U32 srvCounts[] = { 1 };
			static const char* uavNames[] = { "rw_img_mip_shading_change", "rw_img_mip_5", "rw_auto_exposure", "rw_spd_global_atomic" };
			static const U32 uavSlots[] = { 0, 1, 2, 3 };
			static const U32 uavCounts[] = { 1, 1, 1, 1 };
			static const char* samplerNames[] = { "s_LinearClamp" };
			static const U32 samplerSlots[] = { 2 };
			static const U32 samplerCounts[] = { 1 };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				2, 1, 4, 0, 0, 1, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(dev, "FSR2LuminancePyramidCS" + GetGeneralPermutation(fp16, wave64));
			break;
		}
		case FFX_FSR2_PASS_GENERATE_REACTIVE:
		{
			break;
		}
		case FFX_FSR2_PASS_TCR_AUTOGENERATE:
		{
			static const char* cbvNames[] = { "cbFSR2", "cbGenerateReactive" };
			static const U32 cbvSlots[] = { 0, 1 };
			static const U32 cbvCounts[] = { 1, 1 };
			static const char* srvNames[] = { "r_input_color_jittered", "r_input_opaque_only", "r_input_motion_vectors", "r_reactive_mask", "r_transparency_and_composition_mask", "r_input_prev_color_pre_alpha", "r_input_prev_color_post_alpha" };
			static const U32 srvSlots[] = { 0, 1, 2, 3, 4, 5, 6 };
			static const U32 srvCounts[] = { 1, 1, 1, 1, 1, 1, 1 };
			static const char* uavNames[] = { "rw_output_autoreactive", "rw_output_autocomposition", "rw_output_prev_color_pre_alpha", "rw_output_prev_color_post_alpha" };
			static const U32 uavSlots[] = { 0, 1, 2, 3 };
			static const U32 uavCounts[] = { 1, 1, 1, 1, };
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				2, 7, 4, 0, 0, 0, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				nullptr, nullptr, nullptr, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, &BLOB, sizeof(FfxShaderBlob));

			if (shader)
				shader->Init(dev, "FSR2AutoReactiveCS" + getPermutation(false, false, false, false, jitteredMotionVectors, false, fp16, wave64));
			break;
		}
		default:
			ZE_FAIL("Invalid pass for FSR2!");
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