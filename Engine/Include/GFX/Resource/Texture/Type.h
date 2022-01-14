#pragma once

namespace ZE::GFX::Resource::Texture
{
	// Type of single texture
	enum class Type : U8
	{
		Tex2D,
		Tex3D,
		Cube,
		Array
	};
}