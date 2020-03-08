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

	struct Phong
	{
		float specularIntensity;	// The bigger the brighter
		float specularPower;		// The smaller the less focused in one point
		float padding[2]{ 0.0f };
		ColorFloat4 materialColor;
	};

	struct TexPhong
	{
		float specularIntensity;	// The bigger the brighter
		float specularPower;		// The smaller the less focused in one point
		float padding[2]{ 0.0f };
	};

	struct Solid
	{
		ColorFloat4 materialColor;
	};
}