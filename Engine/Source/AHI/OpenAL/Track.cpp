#include "AHI/OpenAL/Track.h"

namespace ZE::AHI::OpenAL
{
	Track::~Track()
	{
		if (source != UINT_MAX)
		{
			alDeleteSources(1, &source);
			ZE_AL_CHECK("Error deleting AL source!");
		}
		if (buffer != UINT_MAX)
		{
			alDeleteBuffers(1, &buffer);
			ZE_AL_CHECK("Error deleting AL buffer!");
		}
	}

	Expected<Track> Track::Create(SFX::Device& dev, const SFX::AudioBuffer& data, const SFX::SoundGroup* group) noexcept
	{
		auto& device = dev.Get().oal;
		Track track;

		alGenBuffers(1, &track.buffer);
		ZE_AL_RET_FAILED_EXPECT();
		ZE_AL_SET_DEBUG_ID(device, AL_BUFFER_EXT, track.buffer, "Track buffer " + std::to_string(data.Desc.Bytes));

		const ALenum format = GetALFormat(device, data.Desc.Channels, data.Desc.BitsPerSample, data.Desc.IsFloat);
		// Don't make copy of a sound inside OpenAL if possible
		if (device.IsExtensionSupported("AL_EXT_STATIC_BUFFER"))
		{
			track.audioData = data.Samples;
			alBufferDataStatic(track.buffer, format, data.Samples.get(), data.Desc.Bytes, data.Desc.SampleRate);
		}
		else
			alBufferData(track.buffer, format, data.Samples.get(), data.Desc.Bytes, data.Desc.SampleRate);
		ZE_AL_RET_FAILED_EXPECT();
		
		alGenSources(1, &track.source);
		ZE_AL_RET_FAILED_EXPECT();
		ZE_AL_SET_DEBUG_ID(device, AL_SOURCE_EXT, track.source, "Track source " + std::to_string(data.Desc.Bytes));

		ZE_CODE_RET_FAILED_EXPECT(SetSource2D(device, track.source));
		return track;
	}

	void Track::Resume() noexcept
	{
		ALenum sourceState = AL_INITIAL;
		alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
		ZE_AL_CHECK("Error checking source playback state!");

		if (sourceState != AL_PLAYING)
		{
			alSourcePlay(source);
			ZE_AL_CHECK("Error playing source!");
		}
	}

	Status Track::Play(U32 loopCount) noexcept
	{
		ALenum sourceState = AL_INITIAL;
		alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
		ZE_AL_RET_FAILED();

		// Play only if it's not being played already
		switch (sourceState)
		{
		case AL_PLAYING:
			break;
		case AL_PAUSED:
		{
			alSourcePlay(source);
			ZE_AL_RET_FAILED();
			break;
		}
		default:
		{
			alSourcei(source, AL_LOOPING, loopCount == UINT32_MAX ? AL_TRUE : AL_FALSE);
			ZE_AL_RET_FAILED();

			if (loopCount && loopCount != UINT32_MAX)
			{
				std::vector<ALuint> unqueuedBuffers(loopCount, buffer);
				alSourceQueueBuffers(source, loopCount, unqueuedBuffers.data());
				ZE_AL_RET_FAILED();
			}
			else
			{
				alSourcei(source, AL_BUFFER, buffer);
				ZE_AL_RET_FAILED();
			}

			alSourcePlay(source);
			ZE_AL_RET_FAILED();
			break;
		}
		}
		return {};
	}

	Status Track::Stop() noexcept
	{
		alSourceStop(source);
		ZE_AL_RET_FAILED();

		// Flush any queued buffers
		alSourcei(source, AL_BUFFER, AL_NONE);
		ZE_AL_RET_FAILED();

		ALint queuedBuffers = 0;
		alGetSourcei(source, AL_BUFFERS_QUEUED, &queuedBuffers);
		ZE_AL_RET_FAILED();
		if (queuedBuffers)
		{
			std::vector<ALuint> unqueuedBuffers(queuedBuffers);
			alSourceUnqueueBuffers(source, queuedBuffers, unqueuedBuffers.data());
			ZE_AL_RET_FAILED();
		}
		return {};
	}
}