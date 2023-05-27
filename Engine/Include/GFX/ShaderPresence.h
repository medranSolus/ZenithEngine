#pragma once
#include <GFX/Resource/ShaderType.h>
#include <bitset>

namespace ZE::GFX
{
	// Mask containing info about currently used shader
	class ShaderPresenceMask final
	{
		std::bitset<6> mask = 0;

	public:
		ShaderPresenceMask() = default;
		ZE_CLASS_DEFAULT(ShaderPresenceMask);
		~ShaderPresenceMask() = default;

		constexpr bool IsVertex() const noexcept { return mask[0]; }
		constexpr bool IsDomain() const noexcept { return mask[1]; }
		constexpr bool IsHull() const noexcept { return mask[2]; }
		constexpr bool IsGeometry() const noexcept { return mask[3]; }
		constexpr bool IsPixel() const noexcept { return mask[4]; }
		constexpr bool IsCompute() const noexcept { return mask[5]; }
		constexpr bool IsGfx() const noexcept { return mask.to_ullong() & GFX::Resource::ShaderType::AllGfx; }

		constexpr void SetVertex() noexcept { mask[0] = true; }
		constexpr void SetDomain() noexcept { mask[1] = true; }
		constexpr void SetHull() noexcept { mask[2] = true; }
		constexpr void SetGeometry() noexcept { mask[3] = true; }
		constexpr void SetPixel() noexcept { mask[4] = true; }
		constexpr void SetCompute() noexcept { mask[5] = true; }
		constexpr void SetGfx() noexcept { mask |= GFX::Resource::ShaderType::AllGfx; }

		// Set shader presence and return true if value has been changed
		constexpr bool SetPresence(GFX::Resource::ShaderTypes type) noexcept;
	};

#pragma region Functions
	constexpr bool ShaderPresenceMask::SetPresence(GFX::Resource::ShaderTypes type) noexcept
	{
		bool alreadyPresent = false;
		if (type & GFX::Resource::ShaderType::Compute)
		{
			alreadyPresent |= IsCompute();
			SetCompute();
		}
		if (type == GFX::Resource::ShaderType::AllGfx)
		{
			alreadyPresent |= IsGfx();
			SetGfx();
			return alreadyPresent;
		}

		if (type & GFX::Resource::ShaderType::Vertex)
		{
			alreadyPresent |= IsVertex();
			SetVertex();
		}
		if (type & GFX::Resource::ShaderType::Domain)
		{
			alreadyPresent |= IsDomain();
			SetDomain();
		}
		if (type & GFX::Resource::ShaderType::Hull)
		{
			alreadyPresent |= IsHull();
			SetHull();
		}
		if (type & GFX::Resource::ShaderType::Geometry)
		{
			alreadyPresent |= IsGeometry();
			SetGeometry();
		}
		if (type & GFX::Resource::ShaderType::Pixel)
		{
			alreadyPresent |= IsPixel();
			SetPixel();
		}
		return alreadyPresent;
	}
#pragma endregion
}