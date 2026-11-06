#pragma once
#include "SFX/StreamDesc.h"

namespace ZE::AHI::OpenAL
{
	class Stream final
	{

	public:
		Stream() = default;
		ZE_CLASS_MOVE(Stream);
		~Stream();

		static Expected<Stream> Create(SFX::Device& dev, const SFX::StreamDesc& desc) noexcept;

		Status SetVolume(float decibels) noexcept { return {}; }
		Status SetPitch(float semitones) noexcept { return {}; }

		Status Resume() noexcept { return {}; }
		Status Pause() noexcept { return {}; }

		bool IsLooping() const noexcept {  }

		Task<Status> ResetPosition(bool autoPlay, bool enableLoop) noexcept;
	};
}