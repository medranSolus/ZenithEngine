#pragma once
#include "Resource/Shader.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_interface.h"
ZE_WARNING_POP

namespace ZE::GFX::FFX
{
	// Get ID for pipeline based on all possible permutations
	U64 GetPipelineID(FfxEffect effect, FfxPass passId, U32 permutationOptions) noexcept;
	// Get information about used shader in pipeline and optionally load shader data
	FfxErrorCode GetShaderInfo(Device* dev, FfxEffect effect, FfxPass pass, U32 permutationOptions, FfxShaderBlob& shaderBlob, Resource::Shader* shader);
}