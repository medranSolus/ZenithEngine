cbuffer PhongBuffer : register(b1)
{
	float3 cb_specularColor;
	float cb_specularIntensity; // The bigger the brighter
	float cb_specularPower;     // The smaller the less focused in one point
#ifdef _TEX
#ifdef _TEX_NORMAL
	float cb_normalMapWeight;
#endif
#ifdef _TEX_SPEC
	bool cb_useSpecularPowerAlpha;
#endif
#else
	float4 cb_materialColor;
#endif
};