#define ZE_DOF_CB_RANGE 3
#include "CB/ConstantsDOF.hlsli"

#define FFX_DOF_OPTION_MAX_MIP 4

UAV2D(bilatColor0, FfxHalfOpt4, 0, 0);
UAV2D(bilatColor1, FfxHalfOpt4, 1, 0);
UAV2D(bilatColor2, FfxHalfOpt4, 2, 0);
UAV2D(bilatColor3, FfxHalfOpt4, 3, 0);
TEXTURE_EX(depthInput, Texture2D<float>, 0, 1);
TEXTURE_EX(colorInput, Texture2D<float4>, 1, 2);

void FfxDofStoreBilatMip(const in FfxUInt32 mip, const in FfxInt32x2 coord, const in FfxHalfOpt4 value)
{
	switch (mip)
	{
	default:
	case 0:
		ua_bilatColor0[coord] = value;
		break;
	case 1:
		ua_bilatColor1[coord] = value;
		break;
	case 2:
		ua_bilatColor2[coord] = value;
		break;
	case 3:
		ua_bilatColor3[coord] = value;
		break;
	}
}

FfxFloat32 FfxDofLoadFullDepth(const in FfxUInt32x2 coord)
{
	return tx_depthInput[coord];
}

FfxHalfOpt4 FfxDofLoadSource(const in FfxInt32x2 coord)
{
	return FfxHalfOpt4(tx_colorInput[coord]);
}

#include "WarningGuardOn.hlsli"
#include "dof/ffx_dof_downsample_color.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(16, 16, 1)]
void main(const uint gidx : SV_GroupIndex, const uint3 gid : SV_GroupID)
{
	DownsampleColor(gidx, gid.xy);
}