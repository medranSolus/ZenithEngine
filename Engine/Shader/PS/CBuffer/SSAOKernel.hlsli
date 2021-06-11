static const uint CB_KERNEL_SIZE = 32;

cbuffer SSAOKernelBuffer : register(b13)
{
	float3 cb_kernel[CB_KERNEL_SIZE];
	float cb_bias;
	float2 cb_tileDimensions;
	float cb_sampleRadius;
	float cb_ssaoPower;
	uint cb_kernelSize;
}