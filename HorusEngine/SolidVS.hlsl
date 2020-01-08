cbuffer TransformConstatBuffer
{
    matrix transform;
    matrix scaling;
    matrix view;
    matrix projection;
};

float4 main(float3 pos : POSITION) : SV_Position
{
    return mul(float4(pos, 1.0f), mul(mul(mul(scaling, transform), view), projection));
}