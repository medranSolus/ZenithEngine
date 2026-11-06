#pragma once
#if _ZE_AHI_OPENAL
#	include "AHI/OpenAL/Stream.h"
#endif
#if _ZE_AHI_XAUDIO2
#	include "AHI/XAudio2/Stream.h"
#endif
#include "AHI/Backend.h"

namespace ZE::SFX
{
	// Single audio track streaming sound directly from a file
	class Stream final
	{
		ZE_AHI_BACKEND(Stream);

	public:
		Stream() = default;
		ZE_CLASS_MOVE(Stream);
		~Stream() = default;

		static Expected<Stream> Create(Device& dev, const StreamDesc& desc) noexcept { ZE_AHI_BACKEND_CREATE(Stream, dev, desc); }
		ZE_AHI_BACKEND_GET(Stream);

		// Main Audio API

		Status SetVolume(float decibels) noexcept { ZE_AHI_BACKEND_CALL_RET(SetVolume, decibels); }
		Status SetPitch(float semitones) noexcept { ZE_AHI_BACKEND_CALL_RET(SetPitch, semitones); }

		Status Resume() noexcept { ZE_AHI_BACKEND_CALL_RET(Resume); }
		Status Pause() noexcept { ZE_AHI_BACKEND_CALL_RET(Pause); }

		bool IsLooping() const noexcept { ZE_AHI_BACKEND_CALL_RET(IsLooping); }

		Task<Status> ResetPosition(bool autoPlay, bool enableLoop) noexcept { ZE_AHI_BACKEND_CALL_RET(ResetPosition, autoPlay, enableLoop); }
	};
}