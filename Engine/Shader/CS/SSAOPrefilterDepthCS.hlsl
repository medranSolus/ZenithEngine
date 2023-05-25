#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#include "Utils/SSAO.hlsli"

RWTexture2D<lpfloat> viewDepthMip0 : register(u0);
RWTexture2D<lpfloat> viewDepthMip1 : register(u1);
RWTexture2D<lpfloat> viewDepthMip2 : register(u2);
RWTexture2D<lpfloat> viewDepthMip3 : register(u3);
RWTexture2D<lpfloat> viewDepthMip4 : register(u4);
TEXTURE_EX(sourceDepthMap, Texture2D<float>, 0);

// XeGTAO first pass
// Each thread computes 2x2 blocks so processing 16x16 block,
// dispatch needs to be called with: (width + 16-1) / 16, (height + 16-1) / 16
[numthreads(8, 8, 1)]
void main(const uint2 dispatchID : SV_DispatchThreadID, const uint2 groupID : SV_GroupThreadID)
{
	XeGTAO_PrefilterDepths16x16(dispatchID, groupID,
		cb_pbrData.SsaoData, tx_sourceDepthMap, splr_PE,
		viewDepthMip0, viewDepthMip1, viewDepthMip2,
		viewDepthMip3, viewDepthMip4);
}