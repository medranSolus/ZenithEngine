#include "AHI/XAudio2/XA2.h"

namespace ZE::AHI::XAudio2
{
	Expected<IXAudio2SourceVoice*> CreateSourceVoice(IXAudio2* dev, const SFX::AudioDesc& desc, IXAudio2SubmixVoice* voiceGroup) noexcept
	{
		ZE_ASSERT(desc.Bytes <= XAUDIO2_MAX_BUFFER_BYTES, "Audio data too large!");

		WAVEFORMATEXTENSIBLE format = {};
		format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		format.Format.nChannels = Intrin::CountBitsSet(static_cast<U32>(desc.Channels));
		format.Format.nSamplesPerSec = desc.SampleRate;
		format.Format.nBlockAlign = format.Format.nChannels * Math::DivideRoundUp<U8>(desc.BitsPerSample, 8);
		format.Format.nAvgBytesPerSec = format.Format.nBlockAlign * desc.SampleRate;
		format.Format.wBitsPerSample = Math::AlignUp<U8>(desc.BitsPerSample, 8);
		format.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
		format.Samples.wValidBitsPerSample = desc.BitsPerSample;
		format.dwChannelMask = desc.Channels; // Channel mask is based on the same values as in WAVEFORMATEXTENSIBLE
		format.SubFormat = desc.IsFloat ? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT : KSDATAFORMAT_SUBTYPE_PCM;

		IXAudio2SourceVoice* src = nullptr;
		XAUDIO2_SEND_DESCRIPTOR sendDest = { 0, voiceGroup };
		XAUDIO2_VOICE_SENDS sendList = { 1, &sendDest };
		ZE_XA2_RET_FAILED_EXPECT(dev->CreateSourceVoice(&src, &format.Format, 0,
			XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, voiceGroup ? &sendList : nullptr, nullptr));
		return src;
	}
}