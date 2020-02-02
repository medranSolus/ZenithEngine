#pragma once
#include "Vertex.h"

namespace GFX::Resource
{
	struct TransformConstatBuffer
	{
		DirectX::XMMATRIX transform;
		DirectX::XMMATRIX scaling;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
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

	struct PhongPixelBuffer
	{
		BasicType::ColorFloat materialColor;
		float specularIntensity;	// The bigger the brighter
		float specularPower;		// The smaller the less focused in one point
		float padding[2]{ 0.0f };
	};

	struct TexPhongPixelBuffer
	{
		float specularIntensity;	// The bigger the brighter
		float specularPower;		// The smaller the less focused in one point
		float padding[2]{ 0.0f };
	};

	struct SolidPixelBuffer
	{
		BasicType::ColorFloat materialColor;
	};
}
