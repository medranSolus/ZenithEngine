cbuffer ViewBuffer : register(b0)
{
	matrix cb_viewProjection[6];
	float3 cb_cameraPos;
}