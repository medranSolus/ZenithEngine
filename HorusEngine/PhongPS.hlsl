#include "Utils.fx"
#include "LightConstantBuffer.fx"
#include "PhongPixelBuffer.fx"

float4 main(float3 viewPos : POSITION, float3 viewNormal : NORMAL) : SV_Target
{
	viewNormal = normalize(viewNormal);
	LightVectorData lightVD = GetLightVectorData(lightPos, viewPos);
	
	const float attenuation = GetAttenuation(atteuationConst, atteuationLinear, attenuationQuad, lightVD.distanceToLight);
	const float3 scaledLightColor = lightColor * lightIntensity / attenuation;
	
	const float3 diffuse = GetDiffuse(scaledLightColor, lightVD.directionToLight, viewNormal);
	const float3 specular = GetSpecular(lightVD.vertexToLight, viewPos, viewNormal, scaledLightColor, specularPower, specularIntensity);

	return float4(saturate((diffuse + ambientColor + specular) * (float3) materialColor), 1.0f);
}