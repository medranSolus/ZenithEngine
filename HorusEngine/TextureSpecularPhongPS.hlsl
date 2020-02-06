#include "Utils.fx"
#include "LightConstantBuffer.fx"
#include "TexPhongPixelBuffer.fx"

Texture2D tex;
Texture2D spec : register(t2);
SamplerState splr;

float4 main(float3 viewPos : POSITION, float3 viewNormal : NORMAL, float2 tc : TEXCOORD) : SV_Target
{
	viewNormal = normalize(viewNormal);
	LightVectorData lightVD = GetLightVectorData(lightPos, viewPos);
	
	const float attenuation = GetAttenuation(atteuationConst, atteuationLinear, attenuationQuad, lightVD.distanceToLight);
	const float3 scaledLightColor = lightColor * lightIntensity / attenuation;
	
	const float3 diffuse = GetDiffuse(scaledLightColor, lightVD.directionToLight, viewNormal);
	
	const float4 specularTex = spec.Sample(splr, tc);
	
	const float specularPower = GetSampledSpecularPower(specularTex);
	const float3 specular = GetSpecular(lightVD.vertexToLight, viewPos, viewNormal, scaledLightColor, specularPower, specularIntensity);

	return float4(saturate((diffuse + ambientColor) * tex.Sample(splr, tc).bgr + specular * specularTex.bgr), 1.0f);
}