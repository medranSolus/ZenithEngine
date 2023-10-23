#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"
#define ZE_XEGTAO_CB_RANGE 3
#include "CB/ConstantsXeGTAO.hlsli"

UAV2D(viewDepthMip0, lpfloat, 0, 1);
UAV2D(viewDepthMip1, lpfloat, 1, 1);
UAV2D(viewDepthMip2, lpfloat, 2, 1);
UAV2D(viewDepthMip3, lpfloat, 3, 1);
UAV2D(viewDepthMip4, lpfloat, 4, 1);
TEXTURE_EX(sourceDepthMap, Texture2D<float>, 0, 2);

// XeGTAO first pass
// Each thread computes 2x2 blocks so processing 16x16 block,
// dispatch needs to be called with: (width + 16-1) / 16, (height + 16-1) / 16
[numthreads(8, 8, 1)]
void main(const uint2 dispatchID : SV_DispatchThreadID, const uint2 groupID : SV_GroupThreadID)
{
	XeGTAO_PrefilterDepths16x16(dispatchID, groupID,
		cb_xegtaoConsts.XeGTAOData, tx_sourceDepthMap, splr_PE,
		ua_viewDepthMip0, ua_viewDepthMip1, ua_viewDepthMip2,
		ua_viewDepthMip3, ua_viewDepthMip4);
}