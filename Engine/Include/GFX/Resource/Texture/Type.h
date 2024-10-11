#pragma once

namespace ZE::GFX::Resource::Texture
{
	// Type of single texture
	enum class Type : U8
	{
		Tex1D,
		Tex1DArray,
		Tex2D,
		Tex2DArray,
		Tex3D,
		Cube,
	};
}