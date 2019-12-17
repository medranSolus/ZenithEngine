#pragma once
#include "Vertex.h"

namespace GFX::Resource
{
	struct TransformConstatBuffer
	{
		DirectX::XMMATRIX modelView;
		DirectX::XMMATRIX modelViewProjection;
	};

	struct LightConstantBuffer
	{
		BasicType::ColorFloat ambientColor;
		BasicType::ColorFloat diffuseColor;
		DirectX::XMFLOAT3 pos;
		float diffuseIntensity;
		float atteuationConst;
		float atteuationLinear;
		float attenuationQuad;
		float padding = 0.0f;
	};

	struct ObjectConstantBuffer
	{
		BasicType::ColorFloat materialColor;
		float specularIntensity;	// The bigger the brighter
		float specularPower;		// The smaller the less focused in one point
		float padding[2] { 0.0f };
	};
}
