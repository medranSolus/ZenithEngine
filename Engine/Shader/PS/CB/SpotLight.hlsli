#include "CBuffer.hlsli"

struct SpotLightBuffer
{
	float3 Color;
	float Intensity;
	float3 Shadow;
	float InnerAngle;
	float3 Direction;
	float OuterAngle;
	float AttnLinear;
	float AttnQuad;
};

CBUFFER(light, SpotLightBuffer, 2);
CBUFFER(lightPos, float3, 1);