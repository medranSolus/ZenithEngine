#pragma once

namespace ZE::GFX::Resource
{
	// Type of single texture
	enum class TextureType : U8
	{
		Tex2D,
		Tex3D,
		Cube,
		Array
	};
}