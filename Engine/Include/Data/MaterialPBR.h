#pragma once
#include "GFX/Material.h"
#include "Entity.h"

namespace ZE::Data
{
	// CPU side flags of PBR material
	typedef U8 PBRFlags;

	// PBR material parameters
	struct MaterialPBR
	{
		static constexpr const char TEX_SCHEMA_NAME[] = "PBR";
		static constexpr const char TEX_COLOR_NAME[] = "texture";
		static constexpr const char TEX_NORMAL_NAME[] = "normal";
		static constexpr const char TEX_SPECULAR_NAME[] = "specular";
		static constexpr const char TEX_HEIGHT_NAME[] = "height";

		enum Flag : PBRFlags { None = 0, UseSpecularPowerAlpha = 1, UseSpecular = 16, UseTexture = 8, UseNormal = 4, UseParallax = 2 };

		ColorF4 Color;
		ColorF3 Specular;
		// Possible flags: None, UseSpecularPowerAlpha
		U32 Flags;
		// The bigger the brighter
		float SpecularIntensity;
		// The smaller the less focused in one point
		float SpecularPower;
		float ParallaxScale;

		// Describes ordering of pipeline states enforced by the flags, pass -1 to get last index for PSO
		static constexpr U8 GetPipelineStateNumber(PBRFlags flags) noexcept;
		static constexpr PBRFlags GetShaderFlagsForState(U8 stateNumber) noexcept;
		static constexpr const wchar_t* DecodeShaderSuffix(PBRFlags flags) noexcept;
	};

	// Component containing graphics material data for meshes
	typedef GFX::Material<MaterialPBR, MaterialPBR::TEX_SCHEMA_NAME> MaterialBuffersPBR;
	// Identifier of single material data
	struct MaterialID { EID ID; };

#pragma region Functions
	constexpr U8 MaterialPBR::GetPipelineStateNumber(PBRFlags flags) noexcept
	{
		// Remove UseSpecularPowerAlpha, as it's used in shader directly
		//
		// Ordering based on bitfield: Specular|Texture|Normal|Parallax
		// Eg: Texture|Specular -> 0b1010 -> 10
		return static_cast<U8>(flags & (UseSpecular | UseTexture | UseNormal | UseParallax)) >> 1;
	}

	constexpr PBRFlags MaterialPBR::GetShaderFlagsForState(U8 stateNumber) noexcept
	{
		// Retrieve original position of flags
		return stateNumber << 1;
	}

	constexpr const wchar_t* MaterialPBR::DecodeShaderSuffix(PBRFlags flags) noexcept
	{
		flags &= ~UseSpecularPowerAlpha;
		switch (flags)
		{
		case None:
			return L"";
		case UseParallax:
			return L"_P";
		case UseSpecular:
			return L"_S";
		case UseSpecular | UseParallax:
			return L"_SP";
		case UseNormal:
			return L"_N";
		case UseNormal | UseParallax:
			return L"_NP";
		case UseNormal | UseSpecular:
			return L"_NS";
		case UseNormal | UseSpecular | UseParallax:
			return L"_NSP";
		case UseTexture:
			return L"_T";
		case UseTexture | UseParallax:
			return L"_TP";
		case UseTexture | UseSpecular:
			return L"_TS";
		case UseTexture | UseSpecular | UseParallax:
			return L"_TSP";
		case UseTexture | UseNormal:
			return L"_TN";
		case UseTexture | UseNormal | UseParallax:
			return L"_TNP";
		case UseTexture | UseNormal | UseSpecular:
			return L"_TNS";
		case UseTexture | UseNormal | UseSpecular | UseParallax:
			return L"_TNSP";
		}
		ZE_FAIL("Unknown set of flags!");
		return L"";
	}
#pragma endregion
}