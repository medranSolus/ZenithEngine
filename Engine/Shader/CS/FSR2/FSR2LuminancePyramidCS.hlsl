#define ZE_FSR2_CB_RANGE 5
#define ZE_SPD_CB_RANGE 6
#include "CB/ConstantsFSR2.hlsli"
#include "CB/ConstantsSPD.hlsli"

UAV2D(imageMipChange, FfxFloat32, 0, 0);
UAV2D(imageMip5, FfxFloat32, 1, 1);
UAV2D(autoExposure, FfxFloat32x2, 2, 2);
UAV2D(spdGlobal, FfxUInt32, 3, 3);
TEXTURE_EX(color, Texture2D<float4>, 0, 4);

void SPD_IncreaseAtomicCounter(inout FfxUInt32 spdCounter)
{
	InterlockedAdd(ua_spdGlobal[FfxInt32x2(0, 0)], 1, spdCounter);
}

void SPD_ResetAtomicCounter()
{
	ua_spdGlobal[FfxInt32x2(0, 0)] = 0;
}

FfxFloat32x3 SampleInputColor(const in FfxFloat32x2 uv)
{
	return tx_color.SampleLevel(splr_LinearClamp, uv, 0).rgb;
}

FfxFloat32x4 SPD_LoadMipmap5(const in FfxInt32x2 pxCoord)
{
	return FfxFloat32x4(ua_imageMip5[pxCoord], 0, 0, 0);
}

void SPD_SetMipmap(const in FfxInt32x2 pxCoord, const in FfxUInt32 slice, const in FfxFloat32 value)
{
	switch (slice)
	{
		case FFX_FSR2_SHADING_CHANGE_MIP_LEVEL:
			ua_imageMipChange[pxCoord] = value;
			break;
		case 5:
			ua_imageMip5[pxCoord] = value;
			break;
		// avoid flattened side effect
		default:
			ua_imageMipChange[pxCoord] = ua_imageMipChange[pxCoord];
			ua_imageMip5[pxCoord] = ua_imageMip5[pxCoord];
			break;
	}
}

FfxFloat32x2 SPD_LoadExposureBuffer()
{
	return ua_autoExposure[FfxInt32x2(0, 0)];
}

void SPD_SetExposureBuffer(const in FfxFloat32x2 value)
{
    ua_autoExposure[FfxInt32x2(0, 0)] = value;
}

#include "fsr2/ffx_fsr2_compute_luminance_pyramid.h"

FFX_PREFER_WAVE64
[numthreads(256, 1, 1)]
void main(const uint3 wgid : SV_GroupID, const uint ltid : SV_GroupIndex)
{
	ComputeAutoExposure(wgid, ltid);
}