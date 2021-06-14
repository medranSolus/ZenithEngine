// Encode normal into modified spherical coordinates
float2 EncodeNormal(const in float3 normal)
{
	/*
		phi = atan(y/x) (if x=0 then y+100)
		theta = z
	*/
	if (normal.x == 0.0f)
		return float2(normal.y + 100.0f, normal.z);
	else
		return float2(atan2(normal.y, normal.x), normal.z);
}

// Decode normal from modified spherical coordinates
float3 DecodeNormal(const in float2 codedNormal)
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

// Reconstruct pixel position from depth buffer
float3 GetWorldPosition(const in float2 texCoord, const in float depth, uniform matrix inverseViewProjection)
{
	const float x = texCoord.x * 2.0f - 1.0f;
	const float y = (1.0f - texCoord.y) * 2.0f - 1.0f;
	const float4 pos = mul(float4(x, y, depth, 1.0f), inverseViewProjection);
	return pos.xyz / pos.w;
}