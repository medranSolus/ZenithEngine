/* Geometry buffer structure for different types of shaders.
	
	tx_color	     <float4> (t29): RGB - color, A = 0.0f: solid; 0.5f: light source; 1.0f: normal
	tx_normal	     <float2> (t30): RG  - normal
	tx_specularColor <float4> (t31): RGB - color, A - power
*/

#ifdef _PS
	Texture2D tx_color         : register(t29);
	Texture2D tx_normal        : register(t30);
	Texture2D tx_specularColor : register(t31);
#elif defined _CS
	Texture2D<float4> tx_color         : register(t29);
	Texture2D<float2> tx_normal        : register(t30);
	Texture2D<float4> tx_specularColor : register(t31);
#else
#error Shader include error! Using GBuffer with wrong type of shader!
#endif