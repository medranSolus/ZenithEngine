#ifndef GBUFFER_UTILS_HLSLI
#define GBUFFER_UTILS_HLSLI

// GBuffer types for declaring storage
typedef float2 CodedNormalGB;
typedef float4 AlbedoGB;
typedef float2 PackedMaterialGB;
typedef float2 MotionGB;
typedef float ReactiveGB;
typedef float TransparencyGB;

// Encode normal into modified spherical coordinates
CodedNormalGB EncodeNormal(const in float3 normal)
{
	/*
		phi = atan(y/x) (if x=0 then y+100)
		theta = z
	*/
	if (normal.x == 0.0f)
		return CodedNormalGB(normal.y + 100.0f, normal.z);
	else
		return CodedNormalGB(atan2(normal.y, normal.x), normal.z);
}

// Decode normal from modified spherical coordinates
float3 DecodeNormal(const in CodedNormalGB codedNormal)
{
	/*
		x = sqrt(1 - z^2) * cos(phi)
		y = sqrt(1 - z^2) * sin(phi)
		z = z
	*/
	if (codedNormal.x > 90.0f)
		return float3(0.0f, codedNormal.x - 100.0f, codedNormal.y);
	else
	{
		float2 sinCosPhi;
		sincos(codedNormal.x, sinCosPhi.x, sinCosPhi.y);
		sinCosPhi *= sqrt(1.0f - codedNormal.y * codedNormal.y);

		return float3(sinCosPhi.y, sinCosPhi.x, codedNormal.y);
	}
}

// Pack material parameters into GBuffer packed format
PackedMaterialGB PackMaterialParams(const in float metalness, const in float roughness)
{
	return float2(metalness, roughness);
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