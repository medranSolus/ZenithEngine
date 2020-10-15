static const uint CB_KERNEL_SIZE = 32;

cbuffer SSAOKernelBuffer : register(b12)
{
	float3 cb_kernel[CB_KERNEL_SIZE];
	float2 cb_tileDimensions;
}