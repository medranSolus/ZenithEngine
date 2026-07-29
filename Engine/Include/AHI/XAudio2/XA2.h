#pragma once
// Headers needed for XAudio2
#include "Platform/WinAPI/ComPtr.h"
#include "Error.h"
ZE_WARNING_PUSH
#define XAUDIO2_HELPER_FUNCTIONS
#include "xapobase.h"
#include "xapofx.h"
#include "xaudio2.h"
#include "xaudio2fx.h"
namespace XDSP
{
	using namespace ZE::External;
}
#include "xdsp.h"
namespace DirectX
{
	typedef Float3 XMFLOAT3;
}
#include "x3daudio.h"
ZE_WARNING_POP

namespace ZE::AHI::XAudio2
{
	// Enable ComPtr for all XAudio2 namespace
	using Platform::WinAPI::ComPtr;

	// Computes sample rate to be in correct format used by the XAudio2 (correct range and with correct step)
	constexpr U32 ComputeSampleRate(U32 sampleRate) noexcept;
	// Get correct volume level accepted by XAudio2
	constexpr float GetVolumeLevel(float decibels) noexcept;
	// Get correct pitch level accepted by XAudio2
	constexpr float GetPitchLevel(float semitones) noexcept;

#pragma region Functions
	constexpr U32 ComputeSampleRate(U32 sampleRate) noexcept
	{
		return Math::Clamp(static_cast<U32>((sampleRate + XAUDIO2_QUANTUM_DENOMINATOR - 1) / XAUDIO2_QUANTUM_DENOMINATOR) * XAUDIO2_QUANTUM_DENOMINATOR,
			static_cast<U32>(XAUDIO2_MIN_SAMPLE_RATE), static_cast<U32>(XAUDIO2_MAX_SAMPLE_RATE));
	}

	constexpr float GetVolumeLevel(float decibels) noexcept
	{
		return Math::Clamp(XAudio2DecibelsToAmplitudeRatio(decibels), -XAUDIO2_MAX_VOLUME_LEVEL, XAUDIO2_MAX_VOLUME_LEVEL);
	}

	constexpr float GetPitchLevel(float semitones) noexcept
	{
		return Math::Clamp(XAudio2SemitonesToFrequencyRatio(semitones), XAUDIO2_MIN_FREQ_RATIO, XAUDIO2_MAX_FREQ_RATIO);
	}
#pragma endregion
}