cbuffer TransformConstatBuffer
{
    matrix modelView;
    matrix modelViewProjection;
};

float4 main(float3 pos : POSITION) : SV_Position
{
	return mul(modelViewProjection, float4(pos, 1.0f));
}