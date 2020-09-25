cbuffer BiasBuffer : register(b1)
{
	uint cb_size;
	float cb_bias;
};

float main(float length : LENGTH) : SV_TARGET
{
	return length + (cb_bias / cb_size);
}