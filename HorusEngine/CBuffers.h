#pragma once
#include "Vertex.h"

namespace GFX::Data::CBuffer
{
	struct Transform
	{
		DirectX::XMMATRIX transformView;
		DirectX::XMMATRIX transformViewProjection;
	};
}