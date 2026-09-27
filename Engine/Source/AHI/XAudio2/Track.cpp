#include "AHI/XAudio2/Track.h"

namespace ZE::AHI::XAudio2
{
	Track::~Track()
	{
		if (sourceVoice)
			sourceVoice->DestroyVoice();
	}

	Expected<Track> Track::Create(SFX::Device& dev, const SFX::AudioBuffer& data, const SFX::SoundGroup* group) noexcept
	{
		Track track;
		track.dataSize = data.Desc.Bytes;
		track.audioData = data.Samples;

		ZE_EXPECT_RET_FAILED(track.sourceVoice, CreateSourceVoice(dev.Get().xa2.GetDevice(), data.Desc, group ? group->Get().xa2.GetVoice() : nullptr));
		return track;
	}

	Status Track::Play(U32 loopCount) noexcept
	{
		XAUDIO2_BUFFER buffer = {};
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		buffer.AudioBytes = dataSize;
		buffer.pAudioData = audioData.get();
		buffer.PlayBegin = 0;
		buffer.PlayLength = 0;
		buffer.LoopBegin = 0;
		buffer.LoopLength = 0;
		buffer.LoopCount = loopCount;
		buffer.pContext = nullptr;
		ZE_XA2_RET_FAILED(sourceVoice->SubmitSourceBuffer(&buffer, nullptr));

		Resume();
		return {};
	}

	Status Track::Stop() noexcept
	{
		Pause();
		ZE_XA2_RET_FAILED(sourceVoice->FlushSourceBuffers());
		return {};
	}
}