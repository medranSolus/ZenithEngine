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
		ZE_CODE_RET_FAILED_CORO(Pause());


		if (autoPlay)
		{
			ZE_CODE_RET_FAILED_CORO(Resume());
		}
		co_return Status{};
	}
}