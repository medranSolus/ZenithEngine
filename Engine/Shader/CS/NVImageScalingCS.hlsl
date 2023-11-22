#include "CB/ConstantsNIS.hlsli"

SamplerState samplerLinearClamp : register(s0);

UAV2D(upscaled, float4, 0, 0);
TEXTURE_EX(input, Texture2D<float4>, 0, 1);
TEXTURE_EX(coefScaler, Texture2D<float4>, 1, 2);
TEXTURE_EX(coefUSM, Texture2D<float4>, 2, 2);

// Defines fixing names of textures used directly in the shader
#define out_texture ua_upscaled
#define in_texture tx_input
#define coef_scaler tx_coefScaler
#define coef_usm tx_coefUSM

#ifndef NIS_HDR_MODE
#	define NIS_HDR_MODE 0
#endif
#ifndef NIS_BLOCK_HEIGHT
#	define NIS_BLOCK_HEIGHT 24
#endif
#ifndef NIS_THREAD_GROUP_SIZE
#	define NIS_THREAD_GROUP_SIZE 256
#endif
// Upscaling
#define NIS_SCALER 1
// Always perform on whole image
#define NIS_VIEWPORT_SUPPORT 0
// Don't clamp output values, this step is performed in tonemapping later on
#define NIS_CLAMP_OUTPUT 0
// Input will never be in NV12 format for compatibility with other types of upscaling
#define NIS_NV12_SUPPORT 0
// Always same block width
#define NIS_BLOCK_WIDTH 32
/* Things modified in the source:
*
* Changed variable 'kEps' to 'epsilon' under NIS_HDR_MODE_LINEAR to avoid clashes with cbuffer variable
*/
#include "NIS_Scaler.hlsli"

[numthreads(NIS_THREAD_GROUP_SIZE, 1, 1)]
void main(const uint3 gid : SV_GroupID, const uint3 tid : SV_GroupThreadID)
{
	NVScaler(gid.xy, tid.x);
}