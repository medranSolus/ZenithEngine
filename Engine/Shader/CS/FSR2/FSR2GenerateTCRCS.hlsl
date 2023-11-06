#define ZE_FSR2_CB_RANGE 11
#define ZE_AUTO_REACTIVE_CB_RANGE 12
#include "CB/ConstantsFSR2.hlsli"
#include "CB/ConstantsAutoReactive.hlsli"

UAV2D(reactive, float, 0, 0);
UAV2D(composition, float, 1, 1);
UAV2D(prevColorPreAlpha, float3, 2, 2);
UAV2D(prevColorPospAlpha, float3, 3, 3);
TEXTURE_EX(colorJittered, Texture2D<float4>, 0, 4); // External resource format
TEXTURE_EX(opaqueOnly, Texture2D<float4>, 1, 5); // External resource format
TEXTURE_EX(motionVectors, Texture2D<float2>, 2, 6); // External resource format
TEXTURE_EX(reactiveMask, Texture2D<FfxFloat32>, 3, 7); // External resource format
TEXTURE_EX(transparencyCompositionMask, Texture2D<FfxFloat32>, 4, 8); // External resource format
TEXTURE_EX(prevColorPreAlpha, Texture2D<float3>, 5, 9);
TEXTURE_EX(prevColorPostAlpha, Texture2D<float3>, 6, 10);

FfxFloat32x3 LoadOpaqueOnly(const in uint2 pxCoord)
{
	return tx_opaqueOnly[pxCoord].xyz;
}

FfxFloat32x3 LoadInputColor(const in FfxUInt32x2 pxCoord)
{
	return tx_colorJittered[pxCoord].rgb;
}

FfxFloat32x3 LoadPrevPreAlpha(const in FFX_MIN16_I2 pxCoord)
{
	return tx_prevColorPreAlpha[pxCoord];
}

FfxFloat32x3 LoadPrevPostAlpha(const in FFX_MIN16_I2 pxCoord)
{
	return tx_prevColorPostAlpha[pxCoord];
}

FfxFloat32x2 LoadInputMotionVector(const in FfxUInt32x2 pxDilatedMotionVectorPos)
{
	FfxFloat32x2 srcMotionVector = tx_motionVectors[pxDilatedMotionVectorPos].xy;
	FfxFloat32x2 uvMotionVector = srcMotionVector * MotionVectorScale();

#if FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS
    uvMotionVector -= MotionVectorJitterCancellation();
#endif
	return uvMotionVector;
}

FfxFloat32 LoadReactiveMask(const in FfxUInt32x2 pxCoord)
{
	return tx_reactiveMask[pxCoord];
}

FfxFloat32 LoadTransparencyAndCompositionMask(const in FfxUInt32x2 pxCoord)
{
	return tx_transparencyCompositionMask[pxCoord];
}

void StoreAutoReactive(const in uint2 pxCoord, const in FFX_MIN16_F2 reactive)
{
	ua_reactive[pxCoord] = reactive.x;
	ua_composition[pxCoord] = reactive.y;
}

void StorePrevPreAlpha(const in uint2 pxCoord, const in FFX_MIN16_F3 color)
{
	ua_prevColorPreAlpha[pxCoord] = color;
}

void StorePrevPostAlpha(const in uint2 pxCoord, const in FFX_MIN16_F3 color)
{
	ua_prevColorPospAlpha[pxCoord] = color;
}

#include "fsr2/ffx_fsr2_tcr_autogen.h"

FFX_PREFER_WAVE64
[numthreads(8, 8, 1)]
void main(const uint2 dtid : SV_DispatchThreadID)
{
    // ToDo: take into account jitter (i.e. add delta of previous jitter and current jitter to previous UV
    // fetch pre- and post-alpha color values
	FFX_MIN16_F2 fUv = (FFX_MIN16_F2(dtid) + FFX_MIN16_F2(0.5f, 0.5f)) / FFX_MIN16_F2(RenderSize());
	FFX_MIN16_F2 fPrevUV = fUv + FFX_MIN16_F2(LoadInputMotionVector(dtid));
    FFX_MIN16_I2 iPrevIdx = FFX_MIN16_I2(fPrevUV * FFX_MIN16_F2(RenderSize()) - 0.5f);

	FFX_MIN16_F3 colorPreAlpha = FFX_MIN16_F3(LoadOpaqueOnly(dtid));
	FFX_MIN16_F3 colorPostAlpha = FFX_MIN16_F3(LoadInputColor(dtid));

    FFX_MIN16_F2 outReactiveMask = 0;
    
	outReactiveMask.y = ComputeTransparencyAndComposition(FFX_MIN16_I2(dtid), iPrevIdx);

    if (outReactiveMask.y > 0.5f)
    {
		outReactiveMask.x = ComputeReactive(FFX_MIN16_I2(dtid), iPrevIdx);
        outReactiveMask.x *= FFX_MIN16_F(ReactiveScale());
        outReactiveMask.x = outReactiveMask.x < ReactiveMax() ? outReactiveMask.x : FFX_MIN16_F(ReactiveMax());
    }

    outReactiveMask.y *= FFX_MIN16_F(TcScale());

	outReactiveMask.x = max(outReactiveMask.x, FFX_MIN16_F(LoadReactiveMask(dtid)));
	outReactiveMask.y = max(outReactiveMask.y, FFX_MIN16_F(LoadTransparencyAndCompositionMask(dtid)));

	StoreAutoReactive(dtid, outReactiveMask);

	StorePrevPreAlpha(dtid, colorPreAlpha);
	StorePrevPostAlpha(dtid, colorPostAlpha);
}