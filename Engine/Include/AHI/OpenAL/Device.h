#pragma once

namespace ZE::AHI::OpenAL
{
	class Device final
	{
	public:
		Device() = default;
		Device(U32 sampleRate);
		ZE_CLASS_DELETE(Device);
		~Device() = default;

		// Audio API Internal

	};
}