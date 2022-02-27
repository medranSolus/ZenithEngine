#pragma once
#include "GFX/Material.h"

namespace ZE::Data
{
	// PBR material parameters
	struct MaterialPBR
	{
		static constexpr const char TEX_SCHEMA_NAME[] = "PBR";

		enum Flag : U32 { UseSpecularPowerAlpha = 1, UseTexture = 2, UseNormal = 4, UseSpecular = 8, UseParallax = 16 };

		ColorF4 Color;
		ColorF3 Specular;
		U32 Flags;
		// The bigger the brighter
		float SpecularIntensity;
		// The smaller the less focused in one point
		float SpecularPower;
		float ParallaxScale;
	};

	// Component containing graphics material data for meshes
	typedef GFX::Material<MaterialPBR, MaterialPBR::TEX_SCHEMA_NAME> MaterialBuffersPBR;
}