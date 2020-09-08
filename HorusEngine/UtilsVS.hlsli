float3 ToShadowSpacePos(const in float3 pos, uniform matrix transform, uniform matrix shadowPos)
{
	return (float3)mul(mul(float4(pos, 1.0f), transform), shadowPos);
}