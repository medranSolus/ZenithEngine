#pragma once
#include "GFX/Resource/IndexBuffer.h"
#include "GFX/Resource/VertexBuffer.h"

namespace ZE::Data
{
	// Component storing data about single geometry
	struct Geometry
	{
		GFX::Resource::VertexBuffer Vertices;
		GFX::Resource::IndexBuffer Indices;
	};
}