#include "AHI/OpenAL/Device.h"

namespace ZE::AHI::OpenAL
{
	Expected<Device> Device::Create(U32 sampleRate) noexcept
	{
		Device dev = {};
		return dev;
	}
}