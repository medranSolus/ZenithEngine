#include "AHI/OpenAL/Stream.h"

namespace ZE::AHI::OpenAL
{
	Stream::~Stream()
	{
	}

	Expected<Stream> Stream::Create(SFX::Device& dev, const SFX::StreamDesc& desc) noexcept
	{
		Stream audioStream;
		return audioStream;
	}

	Task<Status> Stream::ResetPosition(bool autoPlay, bool enableLoop) noexcept
	{
		Pause();


		if (autoPlay)
			Resume();
		co_return Status{};
	}
}