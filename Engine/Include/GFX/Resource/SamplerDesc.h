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
		SamplerFilter Type = 0;
		struct
		{
			Texture::AddressMode U = Texture::AddressMode::Edge;
			Texture::AddressMode V = Texture::AddressMode::Edge;
			Texture::AddressMode W = Texture::AddressMode::Edge;
		} Address;
		float MipLevelBias = 0.0f;
		// Used when Type is SamplerType::Anisotropic, must be in [1; 16] range
		U8 MaxAnisotropy = 1;
		CompareMethod Comparison = CompareMethod::Never;
		Texture::EdgeColor EdgeColor = Texture::EdgeColor::SolidBlack;
		// Must be less or equal to MaxLOD
		float MinLOD = 0.0f;
		// Must be greater or equal to MinLOD. For no limit set to FLT_MAX
		float MaxLOD = FLT_MAX;
		U32 Slot = 0;
	};
}