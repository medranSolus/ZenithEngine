#pragma once

namespace ZE::GFX::Resource
{
	// Possible types of Shaders
	enum class ShaderType : U8
	{
		Vertex,
		Domain,
		Hull,
		Geometry,
		Pixel,
		Compute
	};
}