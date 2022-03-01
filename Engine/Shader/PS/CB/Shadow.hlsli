#include "CBuffer.hlsli"
#include "CB/PhongFlags.hlsli"

CBUFFER(parallaxScale, float, 2);
CBUFFER(materialFlags, uint, 1);
CBUFFER(lightPos, float3, 0);