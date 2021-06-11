/* Lighting buffer structure for different types of shaders.
   Define _LIGHT_PS to only use buffer output structure.

	tx_lighting	<float4> (t27): RGB - light color
	tx_specular	<float4> (t28): RGB - specular color
*/

#ifdef _PS
#ifdef _LIGHT_PS
	struct PSOut
	{
		float4 color    : SV_TARGET0;
		float4 specular : SV_TARGET1;
	};
#else
	Texture2D tx_lighting : register(t27);
	Texture2D tx_specular : register(t28);
#endif
#elif defined _CS
	Texture2D<float4> tx_lighting : register(t27);
	Texture2D<float4> tx_specular : register(t28);
#else
#error Shader include error! Using LightBuffer with wrong type of shader!
#endif