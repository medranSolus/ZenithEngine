#pragma once
#include "Vertex.h"

namespace GFX::Resource
{
	struct TransformConstatBuffer
	{
		DirectX::XMMATRIX transformView;
		DirectX::XMMATRIX transformViewProjection;
	};

	struct LightConstantBuffer
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

	struct PhongPixelBuffer
	{
		BasicType::ColorFloat4 materialColor;
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
		BasicType::ColorFloat4 materialColor;
	};
}
