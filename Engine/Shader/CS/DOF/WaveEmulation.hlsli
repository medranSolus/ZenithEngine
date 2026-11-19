#ifndef WAVE_EMULATION_HLSLI
#define WAVE_EMULATION_HLSLI

// Allows for emulating wave intrinsic for different DX11 targets
#if defined(_ZE_API_DX11) && defined(_ZE_ENABLE_WAVE_EMULATION)
#   define ZE_EMULATED_WAVE_SIZE 32
#   ifndef ZE_EMULATED_WAVE_GTID_NAME
#       define ZE_EMULATED_WAVE_GTID_NAME gtid
#   endif

// Transparent indirection for real intrinsics
#   define WaveGetLaneCount          Emu_WaveGetLaneCount
#   define WaveGetLaneIndex          Emu_WaveGetLaneIndex(ZE_EMULATED_WAVE_GTID_NAME.x)
#   define WaveReadLaneFirst(val)    Emu_WaveReadLaneFirst_F(val, ZE_EMULATED_WAVE_GTID_NAME.x)
#   define WaveReadLaneAt(val, lane) Emu_WaveReadLaneAt_F(val, lane, ZE_EMULATED_WAVE_GTID_NAME.x)
#   define WaveActiveAllTrue(cond)   Emu_WaveActiveAllTrue(cond, ZE_EMULATED_WAVE_GTID_NAME.x)
#   define WaveActiveAnyTrue(cond)   Emu_WaveActiveAnyTrue(cond, ZE_EMULATED_WAVE_GTID_NAME.x)
#   define WaveActiveMax(val)        Emu_WaveActiveMax_F(val, ZE_EMULATED_WAVE_GTID_NAME.x)

// Temporary shared memory for syncs
groupshared float s_WaveEmulFloatMem[1024];
groupshared uint s_WaveEmulBallotMem[32];

uint Emu_WaveGetLaneCount()
{
    return ZE_EMULATED_WAVE_SIZE;
}

uint Emu_WaveGetLaneIndex(const in uint gtidX)
{
    return gtidX % ZE_EMULATED_WAVE_SIZE;
}

float Emu_WaveReadLaneFirst_F(const in float val, const in uint gtidX)
{
    uint waveId = gtidX / ZE_EMULATED_WAVE_SIZE;
    uint laneId = gtidX % ZE_EMULATED_WAVE_SIZE;
    if (laneId == 0)
        s_WaveEmulFloatMem[waveId] = val;
    GroupMemoryBarrierWithGroupSync();
    float res = s_WaveEmulFloatMem[waveId];
    GroupMemoryBarrierWithGroupSync();
    return res;
}

float Emu_WaveReadLaneAt_F(const in float val, const in uint targetLane, const in uint gtidX)
{
    uint waveId = gtidX / ZE_EMULATED_WAVE_SIZE;
    uint laneId = gtidX % ZE_EMULATED_WAVE_SIZE;
    s_WaveEmulFloatMem[waveId * ZE_EMULATED_WAVE_SIZE + laneId] = val;
    GroupMemoryBarrierWithGroupSync();
    float res = s_WaveEmulFloatMem[waveId * ZE_EMULATED_WAVE_SIZE + targetLane];
    GroupMemoryBarrierWithGroupSync();
    return res;
}

bool Emu_WaveActiveAllTrue(const in bool cond, const in uint gtidX)
{
    uint waveId = gtidX / ZE_EMULATED_WAVE_SIZE;
    uint laneId = gtidX % ZE_EMULATED_WAVE_SIZE;
    if (laneId == 0)
        s_WaveEmulBallotMem[waveId] = ~0U;
    GroupMemoryBarrierWithGroupSync();
    if (!cond)
        InterlockedAnd(s_WaveEmulBallotMem[waveId], ~(1U << laneId));
    GroupMemoryBarrierWithGroupSync();
    bool res = (s_WaveEmulBallotMem[waveId] == ~0U);
    GroupMemoryBarrierWithGroupSync();
    return res;
}

bool Emu_WaveActiveAnyTrue(const in bool cond, const in uint gtidX)
{
    uint waveId = gtidX / ZE_EMULATED_WAVE_SIZE;
    uint laneId = gtidX % ZE_EMULATED_WAVE_SIZE;
    if (laneId == 0)
        s_WaveEmulBallotMem[waveId] = 0U;
    GroupMemoryBarrierWithGroupSync();
    if (cond)
        InterlockedOr(s_WaveEmulBallotMem[waveId], (1U << laneId));
    GroupMemoryBarrierWithGroupSync();
    bool res = (s_WaveEmulBallotMem[waveId] != 0U);
    GroupMemoryBarrierWithGroupSync();
    return res;
}

float Emu_WaveActiveMax_F(const in float val, const in uint gtidX)
{
    uint waveId = gtidX / ZE_EMULATED_WAVE_SIZE;
    uint laneId = gtidX % ZE_EMULATED_WAVE_SIZE;
    uint base = waveId * ZE_EMULATED_WAVE_SIZE;

    // Initial value store
    s_WaveEmulFloatMem[base + laneId] = val;
    GroupMemoryBarrierWithGroupSync(); // SAFE: Hit by all threads

    // Parallel reduction tree
    [unroll]
    for (uint offset = ZE_EMULATED_WAVE_SIZE / 2; offset > 0; offset >>= 1)
    {
        if (laneId < offset)
        {
            float other = s_WaveEmulFloatMem[base + laneId + offset];
            if (other > s_WaveEmulFloatMem[base + laneId])
                s_WaveEmulFloatMem[base + laneId] = other;
        }
        GroupMemoryBarrierWithGroupSync(); 
    }

    // Broadcast final max value back to all lanes in the wave
    float maxVal = s_WaveEmulFloatMem[base];
    GroupMemoryBarrierWithGroupSync();
    s_WaveEmulFloatMem[base + laneId] = maxVal;
    GroupMemoryBarrierWithGroupSync();
    
    return s_WaveEmulFloatMem[base + laneId];
}
#endif
#endif // WAVE_EMULATION_HLSLI