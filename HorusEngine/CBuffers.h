#pragma once
#include "Vertex.h"

namespace GFX::Data::CBuffer
{
	struct Transform
	{
		DirectX::XMMATRIX transformView;
		DirectX::XMMATRIX transformViewProjection;
	};

	struct Light
	{
		DirectX::XMFLOAT3 ambientColor;
		float atteuationConst;
		DirectX::XMFLOAT3 lightColor;
		float atteuationLinear;
		DirectX::XMFLOAT3 lightPos;
		float attenuationQuad;
		float lightIntensity;
		float padding[3] = { 0.0f };
	};
}