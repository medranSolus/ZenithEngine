// Encode normal into modified spherical coordinates
float2 EncodeNormal(const in float3 normal)
{
	/*
		phi = atan(y/x)
		theta = z
	*/
	return float2(atan2(normal.y, normal.x) / 3.14159265f, normal.z) * 0.5f + 0.5f;
}

float3 GetMappedNormal(const in float3 bitan, const in float3 normal, const in float2 texcoord,
	uniform Texture2D normalMap, uniform SamplerState splr)
{
	// Make bitangent again orthogonal to normal (Gram-Schmidt process)
	const float3 N = normalize(normal);
	float3 B = normalize(bitan);
	B = normalize(B - dot(B, N) * N);
	// Get rotation from tangent space
	const float3x3 tangentToWorld = float3x3(normalize(cross(N, B)), B, N);
	// Sample normal to tangent space
	const float3 tangentNormal = normalMap.Sample(splr, texcoord).rgb * 2.0f - 1.0f;
	// Transform from tangent into view space
	return normalize(mul(tangentNormal, tangentToWorld));
}