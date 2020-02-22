#pragma once
#include "Vertex.h"

namespace GFX::Resource
{
	struct TransformCBuffer
	{
		DirectX::XMMATRIX transformView;
		DirectX::XMMATRIX transformViewProjection;
	};

	struct LightCBuffer
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

	struct PhongCBuffer
	{
		BasicType::ColorFloat4 materialColor;
		float specularIntensity;	// The bigger the brighter
		float specularPower;		// The smaller the less focused in one point
		float padding[2]{ 0.0f };
	};

	struct TexPhongCBuffer
	{
		float specularIntensity;	// The bigger the brighter
		float specularPower;		// The smaller the less focused in one point
		float padding[2]{ 0.0f };
	};

	struct SolidCBuffer
	{
		BasicType::ColorFloat4 materialColor;
	};
}
