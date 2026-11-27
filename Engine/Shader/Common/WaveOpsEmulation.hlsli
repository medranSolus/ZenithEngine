#ifndef WAVE_OPS_EMULATION_HLSLI
#define WAVE_OPS_EMULATION_HLSLI
// Facilities to emulate support of wave intrinsics on targets that don't support them natively

#ifdef _ZE_API_DX11
#	ifdef _ZE_GPU_AMD
#		include "ags_shader_intrinsics_dx11.hlsl"

// Pure AGS-supported intrinsics with extensions

uint WaveActiveCountBits(bool bBit)
{
	uint2 bits = countbits(AmdDxExtShaderIntrinsics_Ballot(bBit));
	return bits.x + bits.y;
}

bool WaveActiveAnyTrue(bool expr)
{
	return any(AmdDxExtShaderIntrinsics_Ballot(expr) != 0);
}

uint4 WaveActiveBallot(bool expr)
{
	return uint4(AmdDxExtShaderIntrinsics_Ballot(expr), 0, 0);
}

uint WaveGetLaneIndex()
{
	return AmdDxExtShaderIntrinsics_LaneId();
}

float WaveReadLaneFirst(const in float expr)
{
	return AmdDxExtShaderIntrinsics_ReadfirstlaneF(expr);
}

uint WaveReadLaneFirst(const in uint expr)
{
	return AmdDxExtShaderIntrinsics_ReadfirstlaneU(expr);
}

float WaveReadLaneAt(const in float expr, const in uint laneIndex)
{
	return AmdDxExtShaderIntrinsics_ReadlaneF(expr, laneIndex);
}

float2 WaveReadLaneAt(const in float2 expr, const in uint laneIndex)
{
	return float2(WaveReadLaneAt(expr.x, laneIndex), WaveReadLaneAt(expr.y, laneIndex));
}

float3 WaveReadLaneAt(const in float3 expr, const in uint laneIndex)
{
	return float3(WaveReadLaneAt(expr.xy, laneIndex), WaveReadLaneAt(expr.z, laneIndex));
}

float4 WaveReadLaneAt(const in float4 expr, const in uint laneIndex)
{
	return float4(WaveReadLaneAt(expr.xy, laneIndex), WaveReadLaneAt(expr.zw, laneIndex));
}

uint WaveReadLaneAt(const in uint expr, const in uint laneIndex)
{
	return AmdDxExtShaderIntrinsics_ReadlaneU(expr, laneIndex);
}

uint2 WaveReadLaneAt(const in uint2 expr, const in uint laneIndex)
{
	return uint2(WaveReadLaneAt(expr.x, laneIndex), WaveReadLaneAt(expr.y, laneIndex));
}

uint3 WaveReadLaneAt(const in uint3 expr, const in uint laneIndex)
{
	return uint3(WaveReadLaneAt(expr.xy, laneIndex), WaveReadLaneAt(expr.z, laneIndex));
}

uint4 WaveReadLaneAt(const in uint4 expr, const in uint laneIndex)
{
	return uint4(WaveReadLaneAt(expr.xy, laneIndex), WaveReadLaneAt(expr.zw, laneIndex));
}

uint WaveGetLaneCount()
{
	return 64; // Upper bound
}

// Intrinsics depending on others

float QuadReadAcrossDiagonal(const in float localValue)
{
    return WaveReadLaneAt(localValue, WaveGetLaneIndex() ^ 3U);
}

uint QuadReadAcrossDiagonal(const in uint localValue)
{
    return WaveReadLaneAt(localValue, WaveGetLaneIndex() ^ 3U);
}

float QuadReadLaneAt(const in float sourceValue, const in uint quadLaneID)
{
    uint quadBase = WaveGetLaneIndex() & ~3U;
    return WaveReadLaneAt(sourceValue, quadBase + (quadLaneID & 3U));
}

uint QuadReadLaneAt(const in uint sourceValue, const in uint quadLaneID)
{
    uint quadBase = WaveGetLaneIndex() & ~3U;
    return WaveReadLaneAt(sourceValue, quadBase + (quadLaneID & 3U));
}

float QuadReadAcrossX(const in float localValue)
{
    return WaveReadLaneAt(localValue, WaveGetLaneIndex() ^ 1U);
}

uint QuadReadAcrossX(const in uint localValue)
{
    return WaveReadLaneAt(localValue, WaveGetLaneIndex() ^ 1U);
}

float QuadReadAcrossY(const in float localValue)
{
    return WaveReadLaneAt(localValue, WaveGetLaneIndex() ^ 2U);
}

uint QuadReadAcrossY(const in uint localValue)
{
    return WaveReadLaneAt(localValue, WaveGetLaneIndex() ^ 2U);
}

bool WaveActiveAllTrue(bool expr)
{
	return !WaveActiveAnyTrue(!expr);
}

bool WaveActiveAllEqual(const in float expr)
{
	return WaveActiveAllTrue(expr == WaveReadLaneFirst(expr));
}

uint WaveActiveBitAnd(const in uint expr)
{
    uint laneCount = WaveGetLaneCount();
    uint acc = 0xFFFFFFFF;
    [loop]
    for (uint lane = 0; lane < laneCount; ++lane)
        acc &= WaveReadLaneAt(expr, lane);
    return acc;
}

uint WaveActiveBitOr(const in uint expr)
{
    uint laneCount = WaveGetLaneCount();
    uint acc = 0;
    [loop]
    for (uint lane = 0; lane < laneCount; ++lane)
        acc |= WaveReadLaneAt(expr, lane);
    return acc;
}

uint WaveActiveBitXor(const in uint expr)
{
    uint laneCount = WaveGetLaneCount();
    uint acc = 0;
    [loop]
    for (uint lane = 0; lane < laneCount; ++lane)
        acc ^= WaveReadLaneAt(expr, lane);
    return acc;
}

float WaveActiveMax(const in float expr)
{
    uint laneCount = WaveGetLaneCount();
    float result = expr;
    [loop]
    for (uint lane = 0; lane < laneCount; ++lane)
        result = max(result, WaveReadLaneAt(expr, lane));
    return result;
}

uint WaveActiveMax(const in uint expr)
{
    uint laneCount = WaveGetLaneCount();
    uint result = expr;
    [loop]
    for (uint lane = 0; lane < laneCount; ++lane)
        result = max(result, WaveReadLaneAt(expr, lane));
    return result;
}

float WaveActiveMin(const in float expr)
{
    uint laneCount = WaveGetLaneCount();
    float result = expr;
    [loop]
    for (uint lane = 0; lane < laneCount; ++lane)
        result = min(result, WaveReadLaneAt(expr, lane));
    return result;
}

uint WaveActiveMin(const in uint expr)
{
    uint laneCount = WaveGetLaneCount();
    uint result = expr;
    [loop]
    for (uint lane = 0u; lane < laneCount; ++lane)
        result = min(result, WaveReadLaneAt(expr, lane));
    return result;
}

float WaveActiveProduct(const in float expr)
{
    uint laneCount = WaveGetLaneCount();
    float prod = 1.0f;
    [loop]
    for (uint lane = 0; lane < laneCount; ++lane)
        prod *= WaveReadLaneAt(expr, lane);
    return prod;
}

uint WaveActiveProduct(const in uint expr)
{
    uint laneCount = WaveGetLaneCount();
    uint prod = 1;
    [loop]
    for (uint lane = 0; lane < laneCount; ++lane)
        prod *= WaveReadLaneAt(expr, lane);
    return prod;
}

float WaveActiveSum(const in float expr)
{
    uint laneCount = WaveGetLaneCount();
    float sum = 0.0f;
    [loop]
    for (uint lane = 0; lane < laneCount; ++lane)
        sum += WaveReadLaneAt(expr, lane);
    return sum;
}

uint WaveActiveSum(const in uint expr)
{
    uint laneCount = WaveGetLaneCount();
    uint sum = 0;
    [loop]
    for (uint lane = 0; lane < laneCount; ++lane)
        sum += WaveReadLaneAt(expr, lane);
    return sum;
}

uint WavePrefixCountBits(const in bool bBit)
{
    uint laneId = WaveGetLaneIndex();
    uint4 ballot = WaveActiveBallot(bBit);

    uint maskLow  = (laneId < 32) ? ((1U << laneId) - 1) : 0xFFFFFFFF;
    uint maskHigh = (laneId > 32) ? ((1U << (laneId - 32)) - 1) : 0;

    return countbits(ballot.x & maskLow) + countbits(ballot.y & maskHigh);
}

float WavePrefixProduct(const in float value)
{
    uint laneId = WaveGetLaneIndex();
    float prod = 1.0f;
    [loop]
    for (uint lane = 0; lane < laneId; ++lane)
        prod *= WaveReadLaneAt(value, lane);
    return prod;
}

uint WavePrefixProduct(const in uint value)
{
    uint laneId = WaveGetLaneIndex();
    uint prod = 1;
    [loop]
    for (uint lane = 0; lane < laneId; ++lane)
        prod *= WaveReadLaneAt(value, lane);
    return prod;
}

float WavePrefixSum(const in float value)
{
    uint laneId = WaveGetLaneIndex();
    float sum = 0.0f;
    [loop]
    for (uint lane = 0; lane < laneId; ++lane)
        sum += WaveReadLaneAt(value, lane);
    return sum;
}

uint WavePrefixSum(const in uint value)
{
    uint laneId = WaveGetLaneIndex();
    uint sum = 0;
    [loop]
    for (uint lane = 0; lane < laneId; ++lane)
        sum += WaveReadLaneAt(value, lane);
    return sum;
}
#	elif defined(_ZE_GPU_NV)
#       define NV_SHADER_EXTN_SLOT u7
#       include "nvHLSLExtns.h"

// Pure NVAPI-supported intrinsics with extensions

float QuadReadAcrossDiagonal(const in float localValue)
{
    return asfloat(NvShflXor(asint(localValue), 3));
}

uint QuadReadAcrossDiagonal(const in uint localValue)
{
    return asuint(NvShflXor(asint(localValue), 3));
}

float QuadReadAcrossX(const in float localValue)
{
    return asfloat(NvShflXor(asint(localValue), 1));
}

uint QuadReadAcrossX(const in uint localValue)
{
    return asuint(NvShflXor(asint(localValue), 1));
}

float QuadReadAcrossY(const in float localValue)
{
    return asfloat(NvShflXor(asint(localValue), 2));
}

uint QuadReadAcrossY(const in uint localValue)
{
    return asuint(NvShflXor(asint(localValue), 2));
}

uint WaveActiveBitAnd(const in uint expr)
{
    uint v = expr;
    for (uint mask = 16; mask > 0; mask >>= 1)
        v &= asuint(NvShflXor(asint(v), mask));
    return v;
}

uint WaveActiveBitOr(const in uint expr)
{
    uint v = expr;
    for (uint mask = 16; mask > 0; mask >>= 1)
        v |= asuint(NvShflXor(asint(v), mask));
    return v;
}

uint WaveActiveBitXor(const in uint expr)
{
    uint v = expr;
    for (uint mask = 16; mask > 0; mask >>= 1)
        v ^= asuint(NvShflXor(asint(v), mask));
    return v;
}

uint WaveActiveCountBits(bool bBit)
{
    return countbits(NvBallot(bBit));
}

float WaveActiveMax(const in float expr)
{
    float v = expr;
    for (uint mask = 16; mask > 0; mask >>= 1)
        v = max(v, asfloat(NvShflXor(asint(v), mask)));
    return v;
}

uint WaveActiveMax(const in uint expr)
{
    uint v = expr;
    for (uint mask = 16; mask > 0; mask >>= 1)
        v = max(v, asuint(NvShflXor(asint(v), mask)));
    return v;
}

float WaveActiveMin(const in float expr)
{
    float v = expr;
    for (uint mask = 16; mask > 0; mask >>= 1)
        v = min(v, asfloat(NvShflXor(asint(v), mask)));
    return v;
}

uint WaveActiveMin(const in uint expr)
{
    uint v = expr;
    for (uint mask = 16; mask > 0; mask >>= 1)
        v = min(v, asuint(NvShflXor(asint(v), mask)));
    return v;
}

float WaveActiveProduct(const in float expr)
{
    float v = expr;
    for (uint mask = 16; mask > 0; mask >>= 1)
        v *= asfloat(NvShflXor(asint(v), mask));
    return v;
}

uint WaveActiveProduct(const in uint expr)
{
    uint v = expr;
    for (uint mask = 16; mask > 0; mask >>= 1)
        v *= asuint(NvShflXor(asint(v), mask));
    return v;
}

float WaveActiveSum(const in float expr)
{
    float v = expr;
    for (uint mask = 16; mask > 0; mask >>= 1)
        v += asfloat(NvShflXor(asint(v), mask));
    return v;
}

uint WaveActiveSum(const in uint expr)
{
    uint v = expr;
    for (uint mask = 16; mask > 0; mask >>= 1)
        v += asuint(NvShflXor(asint(v), mask));
    return v;
}

bool WaveActiveAllTrue(bool expr)
{
    return NvAll(expr);
}

bool WaveActiveAnyTrue(bool expr)
{
    return NvAny(expr);
}

uint4 WaveActiveBallot(bool expr)
{
    return uint4(NvBallot(expr), 0, 0, 0);
}

uint WaveGetLaneCount()
{
	return NV_WARP_SIZE;
}

uint WaveGetLaneIndex()
{
	return NvGetLaneId();
}

float WaveReadLaneFirst(const in float expr)
{
	return asfloat(NvShfl(asint(expr), 0));
}

uint WaveReadLaneFirst(const in uint expr)
{
	return asuint(NvShfl(asint(expr), 0));
}

float WaveReadLaneAt(const in float expr, const in uint laneIndex)
{
	return asfloat(NvShfl(asint(expr), laneIndex));
}

float2 WaveReadLaneAt(const in float2 expr, const in uint laneIndex)
{
	return asfloat(NvShfl(asint(expr), laneIndex));
}

float3 WaveReadLaneAt(const in float3 expr, const in uint laneIndex)
{
	return asfloat(int3(NvShfl(asint(expr.xy), laneIndex), NvShfl(asint(expr.z), laneIndex)));
}

float4 WaveReadLaneAt(const in float4 expr, const in uint laneIndex)
{
	return asfloat(NvShfl(asint(expr), laneIndex));
}

uint WaveReadLaneAt(const in uint expr, const in uint laneIndex)
{
	return asuint(NvShfl(asint(expr), laneIndex));
}

uint2 WaveReadLaneAt(const in uint2 expr, const in uint laneIndex)
{
	return asuint(NvShfl(asint(expr), laneIndex));
}

uint3 WaveReadLaneAt(const in uint3 expr, const in uint laneIndex)
{
	return asuint(int3(NvShfl(asint(expr.xy), laneIndex), NvShfl(asint(expr.z), laneIndex)));
}

uint4 WaveReadLaneAt(const in uint4 expr, const in uint laneIndex)
{
	return asuint(NvShfl(asint(expr), laneIndex));
}

// Intrinsics depending on others

float QuadReadLaneAt(const in float sourceValue, const in uint quadLaneID)
{
    uint quadBase = WaveGetLaneIndex() & ~3U;
    return WaveReadLaneAt(sourceValue, quadBase + (quadLaneID & 3U));
}

uint QuadReadLaneAt(const in uint sourceValue, const in uint quadLaneID)
{
    uint quadBase = WaveGetLaneIndex() & ~3U;
    return WaveReadLaneAt(sourceValue, quadBase + (quadLaneID & 3U));
}

bool WaveActiveAllEqual(const in float expr)
{
    return NvAll(expr == WaveReadLaneFirst(expr));
}

uint WavePrefixCountBits(const in bool bBit)
{
    uint laneId = WaveGetLaneIndex();
    uint mask = NvBallot(bBit);
    uint lowerMask = (laneId == 0) ? 0 : ((1U << laneId) - 1);
    return countbits(mask & lowerMask);
}

float WavePrefixProduct(const in float value)
{
    uint laneId = WaveGetLaneIndex();
    float inc = value;
    for (uint delta = 1; delta < 32; delta <<= 1)
    {
        float up = asfloat(NvShflUp(asint(inc), delta));
        if (laneId >= delta)
        {
            inc *= up;
        }
    }
    float exc = asfloat(NvShflUp(asint(inc), 1));
    return (laneId == 0) ? 1.0f : exc;
}

uint WavePrefixProduct(const in uint value)
{
    uint laneId = WaveGetLaneIndex();
    uint inc = value;
    for (uint delta = 1; delta < 32; delta <<= 1)
    {
        uint up = asuint(NvShflUp(asint(inc), delta));
        if (laneId >= delta)
            inc *= up;
    }
    uint exc = asuint(NvShflUp(asint(inc), 1));
    return (laneId == 0) ? 1 : exc;
}

float WavePrefixSum(const in float value)
{
    uint laneId = WaveGetLaneIndex();
    float inc = value;
    for (uint delta = 1; delta < 32; delta <<= 1)
    {
        float up = asfloat(NvShflUp(asint(inc), delta));
        if (laneId >= delta)
            inc += up;
    }
    float exc = asfloat(NvShflUp(asint(inc), 1));
    return (laneId == 0) ? 0.0f : exc;
}

uint WavePrefixSum(const in uint value)
{
    uint laneId = WaveGetLaneIndex();
    uint inc = value;
    for (uint delta = 1; delta < 32; delta <<= 1)
    {
        uint up = asuint(NvShflUp(asint(inc), delta));
        if (laneId >= delta)
            inc += up;
    }
    uint exc = asuint(NvShflUp(asint(inc), 1));
    return (laneId == 0) ? 0 : exc;
}
#	else
// No HW specific extensions available, plain fallback required
#		define ZE_DEFAULT_LANE_SIZE 32

float QuadReadAcrossDiagonal(const in float localValue)
{
    return 0.0f;
}

uint QuadReadAcrossDiagonal(const in uint localValue)
{
    return 0;
}

float QuadReadLaneAt(const in float sourceValue, const in uint quadLaneID)
{
    return 0.0f;
}

uint QuadReadLaneAt(const in uint sourceValue, const in uint quadLaneID)
{
    return 0;
}

float QuadReadAcrossX(const in float localValue)
{
    return 0.0f;
}

uint QuadReadAcrossX(const in uint localValue)
{
    return 0;
}

float QuadReadAcrossY(const in float localValue)
{
    return 0.0f;
}

uint QuadReadAcrossY(const in uint localValue)
{
    return 0;
}

bool WaveActiveAllEqual(const in float expr)
{
    return false;
}

uint WaveActiveBitAnd(const in uint expr)
{
    return 0;
}

uint WaveActiveBitOr(const in uint expr)
{
    return 0;
}

uint WaveActiveBitXor(const in uint expr)
{
    return 0;
}

uint WaveActiveCountBits(bool bBit)
{
    return 0;
}

float WaveActiveMax(const in float expr)
{
    return 0.0f;
}

uint WaveActiveMax(const in uint expr)
{
    return 0;
}

float WaveActiveMin(const in float expr)
{
    return 0.0f;
}

uint WaveActiveMin(const in uint expr)
{
    return 0;
}

float WaveActiveProduct(const in float expr)
{
    return 0.0f;
}

uint WaveActiveProduct(const in uint expr)
{
    return 0;
}

float WaveActiveSum(const in float expr)
{
    return 0.0f;
}

uint WaveActiveSum(const in uint expr)
{
    return 0;
}

bool WaveActiveAllTrue(bool expr)
{
    return false;
}

bool WaveActiveAnyTrue(bool expr)
{
    return false;
}

uint4 WaveActiveBallot(bool expr)
{
    return uint4(0, 0, 0, 0);
}

uint WaveGetLaneCount()
{
	return 0;
}

uint WaveGetLaneIndex()
{
	return 0;
}

uint WavePrefixCountBits(const in bool bBit)
{
    return 0;
}

float WavePrefixProduct(const in float value)
{
    return 0.0f;
}

uint WavePrefixProduct(const in uint value)
{
    return 0;
}

float WavePrefixSum(const in float value)
{
    return 0.0f;
}

uint WavePrefixSum(const in uint value)
{
    return 0;
}

float WaveReadLaneFirst(const in float expr)
{
	return 0.0f;
}

uint WaveReadLaneFirst(const in uint expr)
{
	return 0;
}

float WaveReadLaneAt(const in float expr, const in uint laneIndex)
{
	return 0.0f;
}

float2 WaveReadLaneAt(const in float2 expr, const in uint laneIndex)
{
	return 0.0f;
}

float3 WaveReadLaneAt(const in float3 expr, const in uint laneIndex)
{
	return 0.0f;
}

float4 WaveReadLaneAt(const in float4 expr, const in uint laneIndex)
{
	return 0.0f;
}

uint WaveReadLaneAt(const in uint expr, const in uint laneIndex)
{
	return 0;
}

uint2 WaveReadLaneAt(const in uint2 expr, const in uint laneIndex)
{
	return 0;
}

uint3 WaveReadLaneAt(const in uint3 expr, const in uint laneIndex)
{
	return 0;
}

uint4 WaveReadLaneAt(const in uint4 expr, const in uint laneIndex)
{
	return 0;
}
#	endif

bool WaveIsFirstLane()
{
	return WaveGetLaneIndex() == 0;
}
#endif

#endif // WAVE_OPS_EMULATION_HLSLI