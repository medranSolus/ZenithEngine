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
		Device(U32 sampleRate);
		ZE_CLASS_DELETE(Device);
		~Device() = default;

		// Audio API Internal

	};
}