cbuffer SSAOOptionsBuffer : register(b13)
{
	uint cb_kernelSize;
	float cb_sampleRadius;
	float cb_bias;
	float cb_ssaoPower;
}