cbuffer SolidPixelBuffer
{
    float4 materialColor;
};

float4 main() : SV_Target
{
    return materialColor;
}