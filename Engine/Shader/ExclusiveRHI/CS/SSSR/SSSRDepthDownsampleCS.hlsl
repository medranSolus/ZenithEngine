#include "Utils/FFX.hlsli"
#include "Buffers.hlsli"

UAV_EX(spdCounter, globallycoherent RWStructuredBuffer<FfxUInt32>, 0, 0);
// Resources laid out in such fasion due to bug in DXC with resource arrays for SPIR-V generation
UAV2D(depthHierarchy0, FfxFloat32, 1, 1);
UAV2D(depthHierarchy1, FfxFloat32, 2, 1);
UAV2D(depthHierarchy2, FfxFloat32, 3, 1);
UAV2D(depthHierarchy3, FfxFloat32, 4, 1);
UAV2D(depthHierarchy4, FfxFloat32, 5, 1);
UAV2D(depthHierarchy5, FfxFloat32, 6, 1);
UAV2D(depthHierarchy6, FfxFloat32, 7, 1);
UAV2D(depthHierarchy7, FfxFloat32, 8, 1);
UAV2D(depthHierarchy8, FfxFloat32, 9, 1);
UAV2D(depthHierarchy9, FfxFloat32, 10, 1);
UAV2D(depthHierarchy10, FfxFloat32, 11, 1);
UAV2D(depthHierarchy11, FfxFloat32, 12, 1);
UAV2D(depthHierarchy12, FfxFloat32, 13, 1);
TEXTURE_EX(depth, Texture2D<float>, 0, 2); // External resource format

void FFX_SSSR_SPDIncreaseAtomicCounter(inout FfxUInt32 spdCounter)
{
	InterlockedAdd(ua_spdCounter[0], 1, spdCounter);
}

void SpdResetAtomicCounter(const in FfxUInt32 slice)
{
	ua_spdCounter[0] = 0;
}

void FFX_SSSR_WriteDepthHierarchy(const in FfxUInt32 index, const in FfxUInt32x2 coord, const in FfxFloat32 data)
{
	switch (index)
	{
	case 0:
		ua_depthHierarchy0[coord] = data;
		break;
	case 1:
		ua_depthHierarchy1[coord] = data;
		break;
	case 2:
		ua_depthHierarchy2[coord] = data;
		break;
	case 3:
		ua_depthHierarchy3[coord] = data;
		break;
	case 4:
		ua_depthHierarchy4[coord] = data;
		break;
	case 5:
		ua_depthHierarchy5[coord] = data;
		break;
	case 6:
		ua_depthHierarchy6[coord] = data;
		break;
	case 7:
		ua_depthHierarchy7[coord] = data;
		break;
	case 8:
		ua_depthHierarchy8[coord] = data;
		break;
	case 9:
		ua_depthHierarchy9[coord] = data;
		break;
	case 10:
		ua_depthHierarchy10[coord] = data;
		break;
	case 11:
		ua_depthHierarchy11[coord] = data;
		break;
	case 12:
		ua_depthHierarchy12[coord] = data;
		break;
	}
}

#ifdef _ZE_HALF_PRECISION
void SpdStoreH(const in FfxInt32x2 coord, const in FfxFloat16x4 outValue, const in FfxUInt32 index, const in FfxUInt32 slice)
{
    // + 1 as we store a copy of the depth buffer at index 0
	FFX_SSSR_WriteDepthHierarchy(index + 1, coord, (FfxFloat32)outValue.x);
}
#endif

void SpdStore(const in FfxInt32x2 coord, const in FfxFloat32x4 outValue, const in FfxUInt32 index, const in FfxUInt32 slice)
{
	// + 1 as we store a copy of the depth buffer at index 0
	FFX_SSSR_WriteDepthHierarchy(index + 1, coord, outValue.x);
}

void FFX_SSSR_GetDepthHierarchyMipDimensions(const in FfxUInt32 mip, out FfxFloat32x2 imageSize)
{
	switch (mip)
	{
	case 0:
		ua_depthHierarchy0.GetDimensions(imageSize.x, imageSize.y);
		break;
	case 1:
		ua_depthHierarchy1.GetDimensions(imageSize.x, imageSize.y);
		break;
	case 2:
		ua_depthHierarchy2.GetDimensions(imageSize.x, imageSize.y);
		break;
	case 3:
		ua_depthHierarchy3.GetDimensions(imageSize.x, imageSize.y);
		break;
	case 4:
		ua_depthHierarchy4.GetDimensions(imageSize.x, imageSize.y);
		break;
	case 5:
		ua_depthHierarchy5.GetDimensions(imageSize.x, imageSize.y);
		break;
	case 6:
		ua_depthHierarchy6.GetDimensions(imageSize.x, imageSize.y);
		break;
	case 7:
		ua_depthHierarchy7.GetDimensions(imageSize.x, imageSize.y);
		break;
	case 8:
		ua_depthHierarchy8.GetDimensions(imageSize.x, imageSize.y);
		break;
	case 9:
		ua_depthHierarchy9.GetDimensions(imageSize.x, imageSize.y);
		break;
	case 10:
		ua_depthHierarchy10.GetDimensions(imageSize.x, imageSize.y);
		break;
	case 11:
		ua_depthHierarchy11.GetDimensions(imageSize.x, imageSize.y);
		break;
	case 12:
		ua_depthHierarchy12.GetDimensions(imageSize.x, imageSize.y);
		break;
	}
}

#ifdef _ZE_HALF_PRECISION
FfxFloat16x4 SpdLoadH(const in FfxInt32x2 coord, const in FfxUInt32 slice)
{
	// 5 -> 6 as we store a copy of the depth buffer at index 0
	return (FfxFloat16x4)ua_depthHierarchy6[coord].xxxx;
}
#endif

FfxFloat32x4 SpdLoad(const in FfxInt32x2 coord, const in FfxUInt32 slice)
{
	// 5 -> 6 as we store a copy of the depth buffer at index 0
	return ua_depthHierarchy6[coord].xxxx;
}

FfxFloat32 FFX_SSSR_GetInputDepth(FfxUInt32x2 coord)
{
	return tx_depth[coord];
}

void FFX_SSSR_GetInputDepthDimensions(out FfxFloat32x2 imageSize)
{
	tx_depth.GetDimensions(imageSize.x, imageSize.y);
}

#ifdef _ZE_HALF_PRECISION
FfxFloat16x4 SpdLoadSourceImageH(const in FfxInt32x2 coord, const in FfxUInt32 slice)
{
    return (FfxFloat16x4)FFX_SSSR_GetInputDepth(coord).xxxx;
}
#endif

FfxFloat32x4 SpdLoadSourceImage(const in FfxInt32x2 coord, const in FfxUInt32 slice)
{
	return FFX_SSSR_GetInputDepth(coord).xxxx;
}

#include "WarningGuardOn.hlsli"
#include "sssr/ffx_sssr_depth_downsample.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(32, 8, 1)]
void main(const in uint groupIndex : SV_GroupIndex, const in uint3 gid : SV_GroupID, const in uint3 dtid : SV_DispatchThreadID)
{
	DepthDownsample(groupIndex, gid, dtid);
}