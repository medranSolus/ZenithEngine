cbuffer ShadowTransformBuffer : register(b1)
{
	matrix cb_shadowViewProjection;
}

float2 GetShadowUV(const in float3 position)
{
	const float4 shadowSpacePos = mul(float4(position, 1.0f), cb_shadowViewProjection);
	return (shadowSpacePos.xy * float2(0.5f, -0.5f)) / shadowSpacePos.w + float2(0.5f, 0.5f);
}