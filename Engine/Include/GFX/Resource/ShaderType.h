#pragma once

namespace ZE::GFX::Resource
{
	typedef U8 ShaderTypes;
	// Possible types of Shaders
	enum ShaderType : ShaderTypes
	{
		Vertex   = 0x01,
		Domain   = 0x02,
		Hull     = 0x04,
		Geometry = 0x08,
		Pixel    = 0x10,
		AllGfx   = 0x1F,
		Compute  = 0x20,
		All      = 0x3F
	};
}