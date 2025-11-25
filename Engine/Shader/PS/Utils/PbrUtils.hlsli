#ifndef PBR_UTILS_HLSLI
#define PBR_UTILS_HLSLI

static const float PI = 3.14159265f;

// Get Trowbridge-Reitz surface area aligned to the halfway vector between light and surface normal
float GetNormalDistributionTrowbridgeReitzGGX(const in float3 surfaceNormal, const in float3 halfway, const in float roughness)
{
	const float roughness2 = roughness * roughness;
	const float NdotH = max(dot(surfaceNormal, halfway), 0.0f);
	const float NdotH2 = NdotH * NdotH;
	
	const float denominator = (NdotH2 * (roughness2 - 1.0f) + 1.0f);
	return roughness2 / (PI * denominator * denominator);
}

// Get microfacet geometry Schlick GGX term
float GetGeometrySchlickGGX(const in float normalDotDirection, const in float remappedRoughness)
{
	return normalDotDirection / (normalDotDirection * (1.0f - remappedRoughness) + remappedRoughness);
}

// Get Smith's-Schlick microfacet self-shadowing term
float GetSelfShadowingSmithSchlick(const in float3 surfaceNormal, const in float3 directionToCamera, const in float3 directionToLight, const in float roughness)
{
	//const float roughnessIBL = roughness * roughness / 2.0f;
	float roughnessDirect = roughness + 1.0f;
	roughnessDirect = roughnessDirect * roughnessDirect / 8.0f;
	
	const float NdotV = max(dot(surfaceNormal, directionToCamera), 0.0f);
	const float NdotL = max(dot(surfaceNormal, directionToLight), 0.0f);
	
	return GetGeometrySchlickGGX(NdotV, roughnessDirect) * GetGeometrySchlickGGX(NdotL, roughnessDirect);
}

// Get Fresnel-Schlick approximation of reflectivity based on viewing angle
float3 GetFresnelSchlick(const in float3 halfway, const in float3 directionToCamera, const in float3 baseReflectivityF0)
{
	// Tips:
	//  - for most materials in air (n == IOR): F0 = pow((n - 1) / (n + 1), 2)
	//  - in different medium (ex. water), "1" in above equation should be changed to IOR of this medium
	//  - for unknown dielectrics it's okay to use n == 0.04
	//  - for exotic or unreal materials F0 can be in range [0.2, 0.45], otherwise they should be avoided (semiconductors have such values, diamond, etc.)
	//  - metals usually have F0 > 0.5 with different values per channel
	
	// For complex materials (such as metals, but they can work with about equation too) it is needed to modify white color
	// in equation below (part "1.0f - baseReflectivityF0") to better match what is the response of surface at 90 degree angle.	
	return baseReflectivityF0 + (1.0f - baseReflectivityF0) * pow(1.0f - max(dot(halfway, directionToCamera), 0.0f), 5.0f);
}

// Get Fresnel-Schlick approximation of reflectivity based on viewing angle for scattered global lighting
float3 GetFresnelSchlickIBL(const in float3 normal, const in float3 directionToCamera, const in float3 baseReflectivityF0, const in float roughness)
{
	const float revRgh = 1.0f - roughness;
	return baseReflectivityF0 + (max(float3(revRgh, revRgh, revRgh), baseReflectivityF0) - baseReflectivityF0) * pow(1.0f - max(dot(normal, directionToCamera), 0.0f), 5.0f);
}

float3 GetReflectionCookTorrance(const in float3 directionToLight, const in float3 directionToCamera, const in float3 halfwayCameraLight, const in float3 surfaceNormal, const in float3 fresnel, const in float roughness)
{
	const float distribution = GetNormalDistributionTrowbridgeReitzGGX(surfaceNormal, halfwayCameraLight, roughness); // Surface normal distribution for microfacets
	const float geometry = GetSelfShadowingSmithSchlick(surfaceNormal, directionToCamera, directionToLight, roughness); // Geometry self-shadowing

	// Not multiplying by kS since fresnel term represents it (fraction of light reflected)
	return (distribution * geometry / (4.0f * max(dot(surfaceNormal, directionToCamera), 0.0f) * max(dot(surfaceNormal, directionToLight), 0.0f) + 0.0001f)) * fresnel;
}

// Lambert refraction part scaled by reflection/specular component
float3 GetDiffuseLambert(const in float3 surfaceColor, const in float3 fresnel, const in float metalness)
{
	return (float3(1.0f, 1.0f, 1.0f) - fresnel) * surfaceColor * ((1.0f - metalness) / PI);
}

float3 GetBRDFCookTorrance(const in float3 directionToLight, const in float3 directionToCamera, const in float3 surfaceNormal, const in float3 surfaceColor, const in float metalness, const in float roughness)
{
	const float3 halfwayCameraLight = normalize(directionToCamera + directionToLight);
	
#if 0
	// NOT OPTIMIZED VERSION FOR TESTS
	// Surface reflection at different angles, F0 of 0.04 works okay for most dielectricts
	const float3 fresnel = GetFresnelSchlick(halfwayCameraLight, directionToCamera, lerp(0.04f, surfaceColor, metalness));
	const float3 diffuse = GetDiffuseLambert(surfaceColor, fresnel, metalness);
	const float3 reflection = GetReflectionCookTorrance(directionToLight, directionToCamera, halfwayCameraLight, surfaceNormal, fresnel, roughness);
	
	return (diffuse + reflection) * max(dot(surfaceNormal, directionToLight), 0.0f);
#else
	// Fresnel
	const float3 baseReflectivityF0 = lerp(0.04f, surfaceColor, metalness);
	const float3 fresnel = baseReflectivityF0 + (1.0f - baseReflectivityF0) * pow(1.0f - max(dot(halfwayCameraLight, directionToCamera), 0.0f), 5.0f);
	
	// Distribution
	const float roughness2 = roughness * roughness;
	const float NdotH = max(dot(surfaceNormal, halfwayCameraLight), 0.0f);
	const float denominator = (NdotH * NdotH * (roughness2 - 1.0f) + 1.0f);
	
	// Self shadowing
	//const float roughnessIBL = roughness2 / 2.0f;
	float roughnessDirect = roughness + 1.0f;
	roughnessDirect = roughnessDirect * roughnessDirect / 8.0f;
	const float roughnessRemap = roughnessDirect;
	const float roughnessReverse = 1.0f - roughnessRemap;
	
	const float NdotV = max(dot(surfaceNormal, directionToCamera), 0.0);
	const float NdotL = max(dot(surfaceNormal, directionToLight), 0.0);
	
	// Final parts
	// TODO: check if should not divide by 4 in reflection and PI in diffuse, something about point infinitely small...
	const float3 reflection = fresnel * (roughness2 / (PI * denominator * denominator * (4.0f * (NdotV * roughnessReverse + roughnessRemap) * (NdotL * roughnessReverse + roughnessRemap) + 0.0001f)));
	const float3 diffuse = (float3(1.0f, 1.0f, 1.0f) - fresnel) * (surfaceColor * (1.0f - metalness) / PI);
	return (diffuse + reflection) * NdotL;	
#endif
}

#endif // PBR_UTILS_HLSLI