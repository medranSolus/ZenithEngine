
float4 ToShadowSpacePos(const in float3 pos, uniform matrix transform, uniform matrix shadowViewProjection)
{
	const float4 transformPos = mul(float4(pos, 1.0f), transform);
	const float4 shadowSpacePos = mul(transformPos, shadowViewProjection); // <-<-<-<-<---- SHADOW VIEW NO A MADAFUCKING CHANGE!!! IT" THIS SHIT FAULT!!!
	return shadowSpacePos * float4(0.5f, -0.5f, 1.0f, 1.0f) + (float4(0.5f, 0.5f, 0.0f, 0.0f) * shadowSpacePos.w);
}