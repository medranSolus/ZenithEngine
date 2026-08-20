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
		ZE_ASSERT(data.Bytes <= XAUDIO2_MAX_BUFFER_BYTES, "Audio data too large!");
		Track track;
		track.dataSize = data.Bytes;
		track.audioData = data.Samples;

		WAVEFORMATEXTENSIBLE format = {};
		format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		format.Format.nChannels = Intrin::CountBitsSet(static_cast<U32>(data.Channels));
		format.Format.nSamplesPerSec = data.SampleRate;
		format.Format.nBlockAlign = format.Format.nChannels * Math::DivideRoundUp<U8>(data.BitsPerSample, 8);
		format.Format.nAvgBytesPerSec = format.Format.nBlockAlign * data.SampleRate;
		format.Format.wBitsPerSample = Math::AlignUp<U8>(data.BitsPerSample, 8);
		format.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
		format.Samples.wValidBitsPerSample = data.BitsPerSample;
		format.dwChannelMask = data.Channels; // Channel mask is based on the same values as in WAVEFORMATEXTENSIBLE
		format.SubFormat = data.IsFloat ? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT : KSDATAFORMAT_SUBTYPE_PCM;

		XAUDIO2_SEND_DESCRIPTOR sendDest = { 0, group ? group->Get().xa2.GetVoice() : nullptr };
		XAUDIO2_VOICE_SENDS sendList = { 1, &sendDest };
		ZE_XA2_RET_FAILED_EXPECT(dev.Get().xa2.GetDevice()->CreateSourceVoice(&track.sourceVoice, &format.Format, 0,
			XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, group ? &sendList : nullptr, nullptr));

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