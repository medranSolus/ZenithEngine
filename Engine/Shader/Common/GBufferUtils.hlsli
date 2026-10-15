#ifndef GBUFFER_UTILS_HLSLI
#define GBUFFER_UTILS_HLSLI
#include "Utils.hlsli"

// GBuffer types for declaring storage
typedef float2 CodedNormalGB;
typedef float4 AlbedoGB;
typedef float2 PackedMaterialGB;
typedef float2 MotionGB;
typedef float ReactiveGB;
typedef float TransparencyGB;

// Encode normal into proper GBuffer encoding
CodedNormalGB EncodeNormal(const in float3 normal)
{
	return EncodeOctahedral(normal);
}

// Decode normal from GBufferr
float3 DecodeNormal(const in CodedNormalGB codedNormal)
{
	return DecodeOctahedral(codedNormal);
}

// Pack material parameters into GBuffer packed format
PackedMaterialGB PackMaterialParams(const in float metalness, const in float roughness)
{
	return float2(metalness, roughness);
}

// Get material parameters from opaque data format
PackedMaterialGB ExtractMaterialParams(const in float4 data)
{
	return data.rg;
}

// Get metalness component from material parameters
float GetMetalness(const in PackedMaterialGB materialParams)
{
	return materialParams.x;
}

// Get roughness component from material parameters
float GetRoughness(const in PackedMaterialGB materialParams)
{
	return materialParams.y;
}

#endif // GBUFFER_UTILS_HLSLI