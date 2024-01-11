#pragma once
#include "GFX/Material.h"
#include "Entity.h"

namespace ZE::Data
{
	// CPU side flags of PBR material
	struct PBRFlags
	{
		U8 Flags;

		constexpr operator U8& () noexcept { return Flags; }
		constexpr operator const U8& () const noexcept { return Flags; }
	};

	// PBR material parameters
	struct MaterialPBR
	{
		static constexpr const char TEX_SCHEMA_NAME[] = "PHONG";
		static constexpr const char TEX_COLOR_NAME[] = "texture";
		static constexpr const char TEX_NORMAL_NAME[] = "normal";
		static constexpr const char TEX_SPECULAR_NAME[] = "specular";
		static constexpr const char TEX_HEIGHT_NAME[] = "height";

		enum Flag : U8 { None = 0, UseSpecularPowerAlpha = 1, UseSpecular = 16, UseTexture = 8, UseNormal = 4, UseParallax = 2 };

		ColorF4 Color;
		ColorF3 Specular;
		// Possible flags: None, UseSpecularPowerAlpha
		U32 Flags;
		// The bigger the brighter
		float SpecularIntensity;
		// The smaller the less focused in one point
		float SpecularPower;
		float ParallaxScale;

		static constexpr U8 GetLastPipelineStateNumber() noexcept { return GetPipelineStateNumber({ UINT8_MAX }); }
		// Describes ordering of pipeline states enforced by the flags, pass UINT8_MAX to get last index for PSO
		static constexpr U8 GetPipelineStateNumber(PBRFlags flags) noexcept;
		static constexpr PBRFlags GetShaderFlagsForState(U8 stateNumber) noexcept;
		static constexpr const char* DecodeShaderSuffix(PBRFlags flags) noexcept;
	};

	// Component containing graphics material data for meshes
	typedef GFX::Material<MaterialPBR, MaterialPBR::TEX_SCHEMA_NAME> MaterialBuffersPBR;

#pragma region Functions
	constexpr U8 MaterialPBR::GetPipelineStateNumber(PBRFlags flags) noexcept
	{
		// Remove UseSpecularPowerAlpha, as it's used in shader directly
		//
		// Ordering based on bitfield: Specular|Texture|Normal|Parallax
		// Eg: Texture|Specular -> 0b1010 -> 10
		return static_cast<U8>(flags.Flags & (UseSpecular | UseTexture | UseNormal | UseParallax)) >> 1;
	}

	constexpr PBRFlags MaterialPBR::GetShaderFlagsForState(U8 stateNumber) noexcept
	{
		// Retrieve original position of flags
		return { static_cast<U8>(stateNumber << 1) };
	}

	constexpr const char* MaterialPBR::DecodeShaderSuffix(PBRFlags flags) noexcept
	{
		flags.Flags &= ~UseSpecularPowerAlpha;
		switch (flags.Flags)
		{
		case None:
			return "";
		case UseParallax:
			return "_P";
		case UseSpecular:
			return "_S";
		case UseSpecular | UseParallax:
			return "_SP";
		case UseNormal:
			return "_N";
		case UseNormal | UseParallax:
			return "_NP";
		case UseNormal | UseSpecular:
			return "_NS";
		case UseNormal | UseSpecular | UseParallax:
			return "_NSP";
		case UseTexture:
			return "_T";
		case UseTexture | UseParallax:
			return "_TP";
		case UseTexture | UseSpecular:
			return "_TS";
		case UseTexture | UseSpecular | UseParallax:
			return "_TSP";
		case UseTexture | UseNormal:
			return "_TN";
		case UseTexture | UseNormal | UseParallax:
			return "_TNP";
		case UseTexture | UseNormal | UseSpecular:
			return "_TNS";
		case UseTexture | UseNormal | UseSpecular | UseParallax:
			return "_TNSP";
		}
		ZE_FAIL("Unknown set of flags!");
		return "";
	}
#pragma endregion
}