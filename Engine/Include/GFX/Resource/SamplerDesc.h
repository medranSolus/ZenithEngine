#pragma once
#include "Texture/AddressMode.h"
#include "SamplerType.h"

namespace ZE::GFX::Resource
{
	// Function that is used when comparing values in sampler
	enum class CompareMethod : U8 { Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };

	// Definition of Sampler used in shaders
	struct SamplerDesc
	{
		// SamplerType
		U8 Type;
		struct
		{
			Texture::AddressMode U;
			Texture::AddressMode V;
			Texture::AddressMode W;
		} Address;
		float MipLevelBias;
		// Used when Type is SamplerType::Anisotropic, must be in [1; 16] range
		U8 MaxAnisotropy;
		CompareMethod Comparison;
		Texture::EdgeColor EdgeColor;
		// Must be less or equal to MaxLOD
		float MinLOD;
		// Must be greater or equal to MinLOD. For no limit set to FLT_MAX
		float MaxLOD;
		U32 Slot;
	};
}