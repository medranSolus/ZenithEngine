#include "GFX/FfxEffects.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_cacao.h"
#include "../src/components/cacao/ffx_cacao_private.h"
ZE_WARNING_POP

namespace ZE::GFX::FFX
{
	FfxErrorCode GetShaderInfoCACAO(Device& dev, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader);

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
				permutationOptions &= (CACAO_SHADER_PERMUTATION_FORCE_WAVE64 | CACAO_SHADER_PERMUTATION_ALLOW_FP16);
				break;
			case FFX_CACAO_PASS_UPSCALE_BILATERAL_5X5:
				break;
			default:
				ZE_FAIL("Invalid pass for CACAO!");
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

		auto getPermutation = [](bool fp16, bool wave64) -> std::string
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
			};
		auto getPreparePermutation = [](bool downsampled, bool half, bool wave64) -> std::string
			{
				std::string suffix = downsampled || half || wave64 ? "_" : "";
				if (downsampled)
					suffix += "D";
				if (half)
					suffix += "F";
				if (wave64)
					suffix += "W";
				return suffix;
			};

		bool applyNonSmart = false;
		bool half = false;
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
				shader->Init(dev, "CACAOPrepareDepthCS" + getPreparePermutation(prepareDownsampled, half, wave64));
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
				shader->Init(dev, "CACAOPrepareDepthMipsCS" + getPreparePermutation(prepareDownsampled, false, wave64));
			break;
		}
		case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_NORMALS:
		case FFX_CACAO_PASS_PREPARE_NATIVE_NORMALS:
		{
			static const char* srvNames[] = { "g_DepthIn" };
			static const U32 srvSlots[] = { 0 };
			static const U32 srvCounts[] = { 1 };
			static const char* uavNames[] = { "g_RwDeinterleavedNormals" };
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
				shader->Init(dev, "CACAOPrepareNormalsCS" + getPreparePermutation(prepareDownsampled, false, wave64));
			break;
		}
		case FFX_CACAO_PASS_PREPARE_DOWNSAMPLED_NORMALS_FROM_INPUT_NORMALS:
		case FFX_CACAO_PASS_PREPARE_NATIVE_NORMALS_FROM_INPUT_NORMALS:
		{
			static const char* srvNames[] = { "g_NormalIn" };
			static const U32 srvSlots[] = { 0 };
			static const U32 srvCounts[] = { 1 };
			static const char* uavNames[] = { "g_RwDeinterleavedNormals" };
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
				shader->Init(dev, "CACAOPrepareNormalsInputCS" + getPreparePermutation(prepareDownsampled, false, wave64));
			break;
		}
		case FFX_CACAO_PASS_GENERATE_Q0:
		case FFX_CACAO_PASS_GENERATE_Q1:
		case FFX_CACAO_PASS_GENERATE_Q2:
		case FFX_CACAO_PASS_GENERATE_Q3:
		case FFX_CACAO_PASS_GENERATE_Q3_BASE:
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
			static constexpr FfxShaderBlob BLOB =
			{
				nullptr, 0, // Blob, data
				1, 2, 1, 0, 0, 3, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			static constexpr FfxShaderBlob BLOB_Q3 =
			{
				nullptr, 0, // Blob, data
				1, 5, 1, 0, 0, 3, 0, // CBV, SRV tex, UAV tex, SRV buff, UAV buff, samplers, RT
				cbvNames, cbvSlots, cbvCounts, // CBV
				srvNames, srvSlots, srvCounts, // SRV tex
				uavNames, uavSlots, uavCounts, // UAV tex
				nullptr, nullptr, nullptr, // SRV buff
				nullptr, nullptr, nullptr, // UAV buff
				samplerNames, samplerSlots, samplerCounts, // Samplers
				nullptr, nullptr, nullptr, // RT acc
			};
			std::memcpy(&shaderBlob, pass == FFX_CACAO_PASS_GENERATE_Q3 ? &BLOB_Q3 : &BLOB, sizeof(FfxShaderBlob));

			if (shader)
			{
				const U8 qNumber = Utils::SafeCast<U8>(pass - FFX_CACAO_PASS_GENERATE_Q0);
				if (qNumber == 4)
					shader->Init(dev, "CACAOGenerateQ3BaseCS" + getPermutation(false, wave64));
				else
					shader->Init(dev, "CACAOGenerateQ" + std::to_string(qNumber) + "CS" + getPermutation(false, wave64));
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
				shader->Init(dev, "CACAOImportanceMapGenerateCS" + getPermutation(false, wave64));
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
				shader->Init(dev, "CACAOImportanceMapProcessACS" + getPermutation(false, wave64));
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
				shader->Init(dev, "CACAOImportanceMapProcessBCS" + getPermutation(false, wave64));
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
				shader->Init(dev, "CACAOBlur" + std::to_string(pass - FFX_CACAO_PASS_EDGE_SENSITIVE_BLUR_1 + 1) + "CS" + getPermutation(fp16, wave64));
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
				if (applyNonSmart)
				{
					if (half)
						shader->Init(dev, "CACAOApplyNonSmartHalfCS" + getPermutation(false, wave64));
					else
						shader->Init(dev, "CACAOApplyNonSmartCS" + getPermutation(false, wave64));
				}
				else
					shader->Init(dev, "CACAOApplyCS" + getPermutation(false, wave64));
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
				if (upscaleSmartApply)
					shader->Init(dev, "CACAOUpscaleSmartCS" + getPermutation(fp16, wave64));
				else
					shader->Init(dev, "CACAOUpscaleCS" + getPermutation(fp16, wave64));
			}
			break;
		}
		default:
			ZE_FAIL("Invalid pass for CACAO!");
			return FFX_ERROR_INVALID_ENUM;
		}
		return FFX_OK;
	}
}