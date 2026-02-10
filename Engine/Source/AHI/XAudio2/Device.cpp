#include "AHI/XAudio2/Device.h"

namespace ZE::AHI::XAudio2
{
	Expected<Device> Device::Create(U32 sampleRate) noexcept
	{
		// Make sure that COM is initialized
		ZE_WIN_RET_FAILED_EXPECT(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

		Device dev = {};
		ZE_WIN_RET_FAILED_EXPECT(XAudio2Create(&dev.device,
			_ZE_DEBUG_AUDIO_API ? XAUDIO2_DEBUG_ENGINE : 0, XAUDIO2_DEFAULT_PROCESSOR));

#if _ZE_DEBUG_AUDIO_API
		XAUDIO2_DEBUG_CONFIGURATION debugConf = {};
		debugConf.TraceMask = XAUDIO2_LOG_WARNINGS;
		debugConf.BreakMask = XAUDIO2_LOG_ERRORS;
		debugConf.LogThreadID = false;
		debugConf.LogFileline = true;
		debugConf.LogFunctionName = true;
		debugConf.LogTiming = true;
		dev.device->SetDebugConfiguration(&debugConf, nullptr);
#endif
		if (sampleRate != 0)
			sampleRate = Math::Clamp(static_cast<U32>((sampleRate + XAUDIO2_QUANTUM_DENOMINATOR - 1) / XAUDIO2_QUANTUM_DENOMINATOR) * XAUDIO2_QUANTUM_DENOMINATOR,
				static_cast<U32>(XAUDIO2_MIN_SAMPLE_RATE), static_cast<U32>(XAUDIO2_MAX_SAMPLE_RATE));
		else
			sampleRate = XAUDIO2_DEFAULT_SAMPLERATE;
		ZE_WIN_RET_FAILED_EXPECT(dev.device->CreateMasteringVoice(&dev.masterVoice,
			XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, nullptr, nullptr, AudioCategory_GameMedia));
		return dev;
	}
}