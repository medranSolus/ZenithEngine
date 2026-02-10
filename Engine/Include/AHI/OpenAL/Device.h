#pragma once

namespace ZE::AHI::OpenAL
{
	class Device final
	{
	public:
		Device() = default;
		ZE_CLASS_MOVE(Device);
		~Device() = default;

		static Expected<Device> Create(U32 sampleRate) noexcept;

		// Audio API Internal

	};
}