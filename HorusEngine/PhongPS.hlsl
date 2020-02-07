#include "Utils.fx"
#include "LightConstantBuffer.fx"
#include "PhongPixelBuffer.fx"

float4 main(float3 viewPos : POSITION, float3 viewNormal : NORMAL) : SV_Target
{
	clip(materialColor.a < 0.005f ? -1 : 1);
	
	if (dot(viewNormal, viewPos) >= 0.0f)
		viewNormal *= -1.0f;
	viewNormal = normalize(viewNormal);
	LightVectorData lightVD = GetLightVectorData(lightPos, viewPos);
	
	const float attenuation = GetAttenuation(atteuationConst, atteuationLinear, attenuationQuad, lightVD.distanceToLight);
	const float3 scaledLightColor = lightColor * lightIntensity / attenuation;
	
	const float3 diffuse = GetDiffuse(scaledLightColor, lightVD.directionToLight, viewNormal);
	const float3 specular = GetSpecular(lightVD.vertexToLight, viewPos, viewNormal, scaledLightColor, specularPower, specularIntensity);

	return saturate(float4(diffuse + ambientColor + specular, 1.0f) * materialColor);
}