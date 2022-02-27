#include "CBuffer.hlsli"

struct SpotLightBuffer
{
	float3 Color;
	float Intensity;
	float3 Shadow;
	float AttnLinear;
	float3 Direction;
	float AttnQuad;
	float InnerAngle;
	float OuterAngle;
};

CBUFFER(light, SpotLightBuffer, 1);
CBUFFER(lightPos, float3, 0);