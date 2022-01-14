#pragma once

namespace ZE::GFX::Resource::Texture
{
	// Addressing mode of texture accesses, specifying operation when going outside of [0; 1] range
	enum class AddressMode : U8
	{
		// Repeat texture every range
		Repeat,
		// Repeat texture mirroring it
		Mirror,
		// Use color at the edge of the texture
		Edge,
		// Use specified border color
		BorderColor,
		// After mirroring performs Clamp operation
		MirrorOnce
	};

	// Used color when specifying TextureAddressMode::Edge
	enum class EdgeColor : U8
	{
		TransparentBlack,
		SolidBlack,
		SolidWhite
	};
}