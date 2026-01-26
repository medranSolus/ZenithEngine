#ifndef CONSTANTS_LPM_CS_HLSLI
#define CONSTANTS_LPM_CS_HLSLI
#include "Utils/FFX.hlsli"
#include "Buffers.hlsli"

// To correctly use this cbuffer, define 'ZE_LPM_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsLPM
{
	FfxUInt32x4 ctl[24];
	FfxBoolean shoulder;
	FfxBoolean con;
	FfxBoolean soft;
	FfxBoolean con2;
	FfxBoolean clip;
	FfxBoolean scaleOnly;
	FfxUInt32 displayMode;
	FfxUInt32 pad;
};

CBUFFER(lpmConsts, ConstantsLPM, 0, ZE_LPM_CB_RANGE);

FfxUInt32x4 LpmFilterCtl(FfxUInt32 i)
{
	return cb_lpmConsts.ctl[i];
}

FfxBoolean GetShoulder()
{
	return cb_lpmConsts.shoulder;
}

FfxBoolean GetCon()
{
	return cb_lpmConsts.con;
}

FfxBoolean GetSoft()
{
	return cb_lpmConsts.soft;
}

FfxBoolean GetCon2()
{
	return cb_lpmConsts.con2;
}

FfxBoolean GetClip()
{
	return cb_lpmConsts.clip;
}

FfxBoolean GetScaleOnly()
{
	return cb_lpmConsts.scaleOnly;
}

FfxUInt32 GetMonitorDisplayMode()
{
	return cb_lpmConsts.displayMode;
}

#include "lpm/ffx_lpm_resources.h"

#endif // CONSTANTS_LPM_CS_HLSLI