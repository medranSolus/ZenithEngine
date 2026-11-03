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

		void SetVolume(float decibels) noexcept {  }
		void SetPitch(float semitones) noexcept {  }

		void Resume() noexcept {  }
		void Pause() noexcept {  }

		bool IsLooping() const noexcept {  }

		Task<Status> ResetPosition(bool autoPlay, bool enableLoop) noexcept;
	};
}