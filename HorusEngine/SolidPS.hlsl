cbuffer ObjectConstantBuffer
{
    float4 materialColor;
    float specularIntensity;
    float specularPower;
};

float4 main() : SV_Target
{
    return materialColor;
}