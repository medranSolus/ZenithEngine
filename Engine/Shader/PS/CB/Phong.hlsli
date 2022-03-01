#include "CBuffer.hlsli"
#include "PhongFlags.hlsli"

struct PhongBuffer
{
	float4 Color;
	float3 Specular;
	uint Flags;
	float SpecularIntensity; // The bigger the brighter
	float SpecularPower;     // The smaller the less focused in one point
	float ParallaxScale;
};

CBUFFER(material, PhongBuffer, 0);