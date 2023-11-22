#include "CB/ConstantsNIS.hlsli"

SamplerState samplerLinearClamp : register(s0);

UAV2D(upscaled, float4, 0, 0);
TEXTURE_EX(coefScaler, Texture2D, 0, 1);
TEXTURE_EX(coefUSM, Texture2D, 1, 2);

#if NIS_NV12_SUPPORT
TEXTURE_EX(inputY, Texture2D<float>, 2, 3);
TEXTURE_EX(inputUV, Texture2D<float>, 3, 5);
#else
TEXTURE_EX(input, Texture2D<float4>, 2, 3);
#endif

// Defines fixing names of textures used directly in the shader
#define out_texture ua_upscaled
#define coef_scaler tx_coefScaler
#define coef_usm tx_coefUSM
#define in_texture_y tx_inputY
#define in_texture_uv tx_inputUV
#define in_texture tx_input

#ifndef NIS_NV12_SUPPORT
#	define NIS_NV12_SUPPORT 0
#endif
#ifndef NIS_CLAMP_OUTPUT
#	define NIS_CLAMP_OUTPUT 0
#endif
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