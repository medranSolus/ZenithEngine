/* Depth buffer structure for different types of shaders.

	tx_depth <float> (t26): R - depth value
*/

#ifdef _PS
	Texture2D tx_depth : register(t26);
#elif defined _CS
	Texture2D<float> tx_depth : register(t26);
#else
#error Shader include error! Using DepthBuffer with wrong type of shader!
#endif