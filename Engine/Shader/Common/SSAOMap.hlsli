/* SSAO map structure for different types of shaders.

	tx_ssao <float> (t25): R - ambient occlusion value
*/

#ifdef _PS
	Texture2D tx_ssao : register(t25);
#elif defined _CS
	Texture2D<float> tx_ssao : register(t25);
#else
#error Shader include error! Using SSAOMap with wrong type of shader!
#endif