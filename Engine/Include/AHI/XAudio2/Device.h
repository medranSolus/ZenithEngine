#pragma once
#include "XA2.h"

namespace ZE::AHI::XAudio2
{
	class Device final
	{
		ComPtr<IXAudio2> device;
		Ptr<IXAudio2MasteringVoice> masterVoice;

	public:
		Device() = default;
		ZE_CLASS_MOVE(Device);
		~Device() = default;

		static Expected<Device> Create(U32 sampleRate) noexcept;

		void SetVolume(float decibels) noexcept { masterVoice->SetVolume(GetVolumeLevel(decibels), XAUDIO2_COMMIT_NOW); }

		// Audio API Internal

		IXAudio2* GetDevice() const noexcept { return device.Get(); }
	};
}