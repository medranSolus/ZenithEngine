#pragma once
#include "XA2.h"

namespace ZE::AHI::XAudio2
{
	class Device final
	{
		ComPtr<IXAudio2> device = nullptr;
		Ptr<IXAudio2MasteringVoice> masterVoice = nullptr;

	public:
		Device() = default;
		ZE_CLASS_MOVE(Device);
		~Device() = default;

		static Expected<Device> Create(U32 sampleRate) noexcept;

		// Audio API Internal

	};
}