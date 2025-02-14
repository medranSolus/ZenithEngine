#define ZE_FSR3_CB_RANGE 3
#define ZE_REACTIVE_CB_RANGE 4
#include "CB/ConstantsFSR3.hlsli"
#include "CB/ConstantsReactive.hlsli"

UAV2D(autoReactive, float, 0, 0);
TEXTURE_EX(colorJittered, Texture2D<float4>, 0, 1); // External resource format
TEXTURE_EX(opaqueOnly, Texture2D<float4>, 1, 2); // External resource format

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint2 dtid : SV_DispatchThreadID)
{
	float3 ColorPreAlpha = tx_opaqueOnly[dtid].rgb;
	float3 ColorPostAlpha = tx_colorJittered[dtid].rgb;
    
	if (GenReactiveFlags() & FFX_FSR3UPSCALER_AUTOREACTIVEFLAGS_APPLY_TONEMAP)
	{
		ColorPreAlpha = Tonemap(ColorPreAlpha);
		ColorPostAlpha = Tonemap(ColorPostAlpha);
	}
	
	if (GenReactiveFlags() & FFX_FSR3UPSCALER_AUTOREACTIVEFLAGS_APPLY_INVERSETONEMAP)
	{
		ColorPreAlpha = InverseTonemap(ColorPreAlpha);
		ColorPostAlpha = InverseTonemap(ColorPostAlpha);
	}
	
	float out_reactive_value = 0.f;
	float3 delta = abs(ColorPostAlpha - ColorPreAlpha);
    
	out_reactive_value = (GenReactiveFlags() & FFX_FSR3UPSCALER_AUTOREACTIVEFLAGS_USE_COMPONENTS_MAX) ? max(delta.x, max(delta.y, delta.z)) : length(delta);
	out_reactive_value *= GenReactiveScale();
	
	out_reactive_value = (GenReactiveFlags() & FFX_FSR3UPSCALER_AUTOREACTIVEFLAGS_APPLY_THRESHOLD) ? (out_reactive_value < GenReactiveThreshold() ? 0 : GenReactiveBinaryValue()) : out_reactive_value;

	ua_autoReactive[dtid] = out_reactive_value;
}