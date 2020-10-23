cbuffer CameraBuffer : register(b7)
{
	matrix cb_viewProjection;
	matrix cb_inverseViewProjection;
	float3 cb_cameraPos;
	float cb_nearClip;
	float cb_farClip;
};