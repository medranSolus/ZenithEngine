#pragma once
#include "GFX/Material.h"
#include "Entity.h"

namespace ZE::Data
{
	// PBR material parameters
	struct MaterialPBR
	{
		static constexpr const char TEX_SCHEMA_NAME[] = "PBR";
		static constexpr const char TEX_COLOR_NAME[] = "texture";
		static constexpr const char TEX_NORMAL_NAME[] = "normal";
		static constexpr const char TEX_SPECULAR_NAME[] = "specular";
		static constexpr const char TEX_HEIGHT_NAME[] = "height";

		enum Flag : U32 { None = 0, UseSpecularPowerAlpha = 1, UseTexture = 16, UseNormal = 8, UseSpecular = 4, UseParallax = 2 };

		ColorF4 Color;
		ColorF3 Specular;
		U32 Flags;
		// The bigger the brighter
		float SpecularIntensity;
		// The smaller the less focused in one point
		float SpecularPower;
		float ParallaxScale;

		// Describes ordering of pipeline states enforced by the flags, pass -1 to get last index for PSO
		static constexpr U8 GetPipelineStateNumber(U32 flags) noexcept;
		static constexpr U32 GetShaderFlagsForState(U8 stateNumber) noexcept;
		static constexpr const wchar_t* DecodeShaderSuffix(U32 flags) noexcept;
	};

	// Component containing graphics material data for meshes
	typedef GFX::Material<MaterialPBR, MaterialPBR::TEX_SCHEMA_NAME> MaterialBuffersPBR;
	// Identifier of single material data
	struct MaterialID { EID ID; U8 StateIndex; };

#pragma region Functions
	constexpr U8 MaterialPBR::GetPipelineStateNumber(U32 flags) noexcept
	{
		// Remove UseSpecularPowerAlpha, as it's used in shader directly
		//
		// Ordering based on bitfield: Texture|Normal|Specular|Parallax
		// Eg: Texture|Specular -> 0b1010 -> 10
		return static_cast<U8>(flags & (UseTexture | UseNormal | UseSpecular | UseParallax)) >> 1;
	}

	constexpr U32 MaterialPBR::GetShaderFlagsForState(U8 stateNumber) noexcept
	{
		// Retrieve original position of flags
		return stateNumber << 1;
	}

	constexpr const wchar_t* MaterialPBR::DecodeShaderSuffix(U32 flags) noexcept
	{
		flags &= ~UseSpecularPowerAlpha;
		if (flags == None)
			return L"";
		if (flags == UseParallax)
			return L"_P";
		if (flags == UseSpecular)
			return L"_S";
		if (flags == (UseSpecular | UseParallax))
			return L"_SP";
		if (flags == UseNormal)
			return L"_N";
		if (flags == (UseNormal | UseParallax))
			return L"_NP";
		if (flags == (UseNormal | UseSpecular))
			return L"_NS";
		if (flags == (UseNormal | UseSpecular | UseParallax))
			return L"_NSP";
		if (flags == UseTexture)
			return L"_T";
		if (flags == (UseTexture | UseParallax))
			return L"_TP";
		if (flags == (UseTexture | UseSpecular))
			return L"_TS";
		if (flags == (UseTexture | UseSpecular | UseParallax))
			return L"_TSP";
		if (flags == (UseTexture | UseNormal))
			return L"_TN";
		if (flags == (UseTexture | UseNormal | UseParallax))
			return L"_TNP";
		if (flags == (UseTexture | UseNormal | UseSpecular))
			return L"_TNS";
		if (flags == (UseTexture | UseNormal | UseSpecular | UseParallax))
			return L"_TNSP";
		ZE_ASSERT(false, "Unknown set of flags!");
		return L"";
	}
#pragma endregion
}