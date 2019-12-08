#pragma once
#include "BasicTypes.h"

namespace GFX::Resource
{
	struct TransformConstatBuffer
	{
		DirectX::XMMATRIX modelView;
		DirectX::XMMATRIX modelViewProjection;
	};

	struct LightConstantBuffer
	{
		Primitive::Color ambientColor;
		Primitive::Color diffuseColor;
		DirectX::XMFLOAT3 pos;
		float diffuseIntensity;
		float atteuationConst;
		float atteuationLinear;
		float attenuationQuad;
		float padding = 0.0f;
	};

	struct ObjectConstantBuffer
	{
		Primitive::Color materialColor;
		float specularIntensity;
		float specularPower;
		float padding[2] { 0.0f };
	};
}
