#include "CBuffer.hlsli"

static const uint FLAG_USE_SPECULAR_POWER_ALPHA = 1;
static const uint FLAG_USE_TEXTURE = 2;
static const uint FLAG_USE_NORMAL = 4;
static const uint FLAG_USE_SPECULAR = 8;
static const uint FLAG_USE_PARALLAX = 16;

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