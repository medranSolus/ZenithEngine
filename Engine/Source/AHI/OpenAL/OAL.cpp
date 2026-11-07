#include "AHI/OpenAL/OAL.h"
#include "AHI/OpenAL/Stream.h"

namespace ZE::AHI::OpenAL
{
#pragma region Extension functions
	LPALDEBUGMESSAGECALLBACKEXT alDebugMessageCallbackEXT = nullptr;
	LPALOBJECTLABELEXT alObjectLabelEXT = nullptr;
	PFNALBUFFERDATASTATICPROC alBufferDataStatic = nullptr;
	LPALEVENTCONTROLSOFT alEventControlSOFT = nullptr;
	LPALEVENTCALLBACKSOFT alEventCallbackSOFT = nullptr;

	void LoadExtensions(Device& dev) noexcept
	{
		if (dev.IsExtensionSupported("AL_EXT_debug"))
		{
			alDebugMessageCallbackEXT = reinterpret_cast<LPALDEBUGMESSAGECALLBACKEXT>(alGetProcAddress("alDebugMessageCallbackEXT"));
			alObjectLabelEXT = reinterpret_cast<LPALOBJECTLABELEXT>(alGetProcAddress("alObjectLabelEXT"));
		}
		if (dev.IsExtensionSupported("AL_EXT_STATIC_BUFFER"))
		{
			alBufferDataStatic = reinterpret_cast<PFNALBUFFERDATASTATICPROC>(alGetProcAddress("alBufferDataStatic"));
		}
		if (dev.IsExtensionSupported("AL_SOFT_events"))
		{
			alEventControlSOFT = reinterpret_cast<LPALEVENTCONTROLSOFT>(alGetProcAddress("alEventControlSOFT"));
			alEventCallbackSOFT = reinterpret_cast<LPALEVENTCALLBACKSOFT>(alGetProcAddress("alEventCallbackSOFT"));
		}
	}
#pragma endregion

	ALenum GetALFormat(Device& dev, SFX::ChannelMask channels, U8 bitsPerSample, bool isFloat) noexcept
	{
		ZE_ASSERT(dev.IsExtensionSupported("AL_EXT_float32") || !isFloat,
			"Required OpenAL AL_EXT_float32 extension is not available!");

		bitsPerSample = Math::AlignUp<U8>(bitsPerSample, 8);
		switch (channels)
		{
		case Base(SFX::SoundChannel::Mono):
		{
			switch (bitsPerSample)
			{
			case 8:
				return AL_FORMAT_MONO8;
			case 16:
				return AL_FORMAT_MONO16;
			case 32:
			{
				if (isFloat)
					return AL_FORMAT_MONO_FLOAT32;
				break;
			}
			default:
				break;
			}
			break;
		}
		case Base(SFX::SoundChannel::Stereo_2_0):
		{
			switch (bitsPerSample)
			{
			case 8:
				return AL_FORMAT_STEREO8;
			case 16:
				return AL_FORMAT_STEREO16;
			case 32:
			{
				if (isFloat)
					return AL_FORMAT_STEREO_FLOAT32;
				break;
			}
			default:
				break;
			}
			break;
		}
		default:
			break;
		}

		// Multi-channel formats
		if (dev.IsExtensionSupported("AL_EXT_MCFORMATS"))
		{
			switch (channels)
			{
			case Base(SFX::SoundChannel::Stereo_4_0):
			{
				switch (bitsPerSample)
				{
				case 8:
					return AL_FORMAT_QUAD8;
				case 16:
					return AL_FORMAT_QUAD16;
				case 32:
				{
					if (isFloat)
						return AL_FORMAT_QUAD32;
					break;
				}
				default:
					break;
				}
				break;
			}
			case Base(SFX::SoundChannel::Stereo_5_1):
			{
				switch (bitsPerSample)
				{
				case 8:
					return AL_FORMAT_51CHN8;
				case 16:
					return AL_FORMAT_51CHN16;
				case 32:
				{
					if (isFloat)
						return AL_FORMAT_51CHN32;
					break;
				}
				default:
					break;
				}
				break;
			}
			case Base(SFX::SoundChannel::Stereo_6_1):
			{
				switch (bitsPerSample)
				{
				case 8:
					return AL_FORMAT_61CHN8;
				case 16:
					return AL_FORMAT_61CHN16;
				case 32:
				{
					if (isFloat)
						return AL_FORMAT_61CHN32;
					break;
				}
				default:
					break;
				}
				break;
			}
			case Base(SFX::SoundChannel::Stereo_7_1):
			{
				switch (bitsPerSample)
				{
				case 8:
					return AL_FORMAT_71CHN8;
				case 16:
					return AL_FORMAT_71CHN16;
				case 32:
				{
					if (isFloat)
						return AL_FORMAT_71CHN32;
					break;
				}
				default:
					break;
				}
				break;
			}
			case SFX::SoundChannel::BackLeft | SFX::SoundChannel::BackRight:
			{
				switch (bitsPerSample)
				{
				case 8:
					return AL_FORMAT_REAR8;
				case 16:
					return AL_FORMAT_REAR16;
				case 32:
				{
					if (isFloat)
						return AL_FORMAT_REAR32;
					break;
				}
				default:
					break;
				}
				break;
			}
			default:
				break;
			}
		}

		// Rare 64 bit format
		if (dev.IsExtensionSupported("AL_EXT_double"))
		{
			if (bitsPerSample == 64 && isFloat)
			{
				switch (channels)
				{
				case Base(SFX::SoundChannel::Mono):
					return AL_FORMAT_MONO_DOUBLE_EXT;
				case Base(SFX::SoundChannel::Stereo_2_0):
					return AL_FORMAT_STEREO_DOUBLE_EXT;
				default:
					break;
				}
			}
		}
		ZE_FAIL("Unsupported channels configuration for OpenAL!");
		return UINT_MAX;
	}

	Status SetSource2D(Device& dev, ALuint source) noexcept
	{
		alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
		ZE_AL_RET_FAILED();

		// Disable source spatialization since this is 2D sound
		if (dev.IsExtensionSupported("AL_SOFT_source_spatialize"))
		{
			alSourcei(source, AL_SOURCE_SPATIALIZE_SOFT, AL_FALSE);
			ZE_AL_CHECK("Error disabling source spatialization!");
		}

		// Send audio directly with as little virtualization as possible
		if (dev.IsExtensionSupported("AL_SOFT_direct_channels") && dev.IsExtensionSupported("AL_SOFT_direct_channels_remix"))
		{
			alSourcei(source, AL_DIRECT_CHANNELS_SOFT, AL_REMIX_UNMATCHED_SOFT);
			ZE_AL_CHECK("Error specifying direct channels for track!");
		}
		return {};
	}

	Status ResumePlayback(ALuint source) noexcept
	{
		ALenum sourceState = AL_INITIAL;
		alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
		ZE_AL_RET_FAILED();

		if (sourceState != AL_PLAYING)
		{
			alSourcePlay(source);
			ZE_AL_RET_FAILED();
		}
		return {};
	}

	void EventCallback(ALenum eventType, ALuint object, ALuint param,
		ALsizei length, const ALchar* message, void* userParam) noexcept
	{
		switch (eventType)
		{
		case AL_EVENT_TYPE_BUFFER_COMPLETED_SOFT:
		{
			// If it's stream object then signal need for new buffer in the queue
			Stream::OnBufferEndCallback(object, Utils::SafeCast<U8>(param));
			break;
		}
		default:
			break;
		}
	}
}