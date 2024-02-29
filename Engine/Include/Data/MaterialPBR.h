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

#pragma pack(push, 1)
	// PBR material parameters
	struct MaterialPBR
	{
		static constexpr const char TEX_SCHEMA_NAME[] = "PBR";
		static constexpr const char TEX_ALBEDO_NAME[] = "albedo";
		static constexpr const char TEX_NORMAL_NAME[] = "normal";
		static constexpr const char TEX_METAL_NAME[] = "metalness";
		static constexpr const char TEX_ROUGH_NAME[] = "roughness";
		static constexpr const char TEX_HEIGHT_NAME[] = "height";

		enum Flag : U8
		{
			None = 0,
			UseAlbedoTex = 1,
			UseNormalTex = 2,
			UseMetalnessTex = 4,
			UseRoughnessTex = 8,
			UseParallaxTex = 16,
			IsTransparent = 32,
			// Mask to indicate which flags contribute to physical permutations of shader
			PermutationMask = UseParallaxTex | IsTransparent
		};

		ColorF4 Albedo;
		float Metalness;
		float Roughness;
		float ParallaxScale;
		U32 Flags;

		static constexpr U8 GetLastPipelineStateNumber() noexcept { return GetPipelineStateNumber({ UINT8_MAX }); }
		// Describes ordering of pipeline states enforced by the flags, pass UINT8_MAX to get last index for PSO
		static constexpr U8 GetPipelineStateNumber(PBRFlags flags) noexcept;
		static constexpr PBRFlags GetShaderFlagsForState(U8 stateNumber) noexcept;
		static constexpr const char* DecodeShaderSuffix(PBRFlags flags) noexcept;
	};
#pragma pack(pop)

	// Component containing graphics material data for meshes
	typedef GFX::Material<MaterialPBR, MaterialPBR::TEX_SCHEMA_NAME> MaterialBuffersPBR;

#pragma region Functions
	constexpr U8 MaterialPBR::GetPipelineStateNumber(PBRFlags flags) noexcept
	{
		// Remove UseSpecularPowerAlpha, as it's used in shader directly
		//
		// Ordering based on bitfield: Parallax|Transparent
		// Eg: Texture|Specular -> 0b1010 -> 10
		return static_cast<U8>(flags.Flags & Flag::PermutationMask) >> 4;
	}

	constexpr PBRFlags MaterialPBR::GetShaderFlagsForState(U8 stateNumber) noexcept
	{
		// Retrieve original position of flags
		return { static_cast<U8>(stateNumber << 4) };
	}

	constexpr const char* MaterialPBR::DecodeShaderSuffix(PBRFlags flags) noexcept
	{
		flags.Flags &= ~Flag::PermutationMask;
		switch (flags.Flags)
		{
		case Flag::None:
			return "";
		case Flag::IsTransparent:
			return "_T";
		case Flag::UseParallaxTex:
			return "_P";
		case Flag::IsTransparent | Flag::UseParallaxTex:
			return "_TP";
		}
		ZE_FAIL("Unknown set of flags!");
		return "";
	}
#pragma endregion
}