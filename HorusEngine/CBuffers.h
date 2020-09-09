#pragma once

namespace GFX::Data::CBuffer
{
	struct Transform
	{
		DirectX::XMMATRIX transform;
		DirectX::XMMATRIX transformView;
		DirectX::XMMATRIX transformViewProjection;
	};
}