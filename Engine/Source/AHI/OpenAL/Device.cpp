#include "AHI/OpenAL/Device.h"

namespace ZE::AHI::OpenAL
{
	void Device::MessageCallback(ALenum source, ALenum type, ALuint id, ALenum severity,
		ALsizei length, const ALchar* message, void* userParam) noexcept
	{
#if _ZE_DEBUG_AUDIO_API
		// Format message properly
		std::string msg;

		switch (source)
		{
		case AL_DEBUG_SOURCE_API_EXT:
		{
			msg = "[Source: AL runtime] ";
			break;
		}
		case AL_DEBUG_SOURCE_AUDIO_SYSTEM_EXT:
		{
			msg = "[Source: ALC layer] ";
			break;
		}
		case AL_DEBUG_SOURCE_THIRD_PARTY_EXT:
		{
			msg = "[Source: 3rd party] ";
			break;
		}
		case AL_DEBUG_SOURCE_APPLICATION_EXT:
		{
			msg = "[Source: app] ";
			break;
		}
		case AL_DEBUG_SOURCE_OTHER_EXT:
		{
			msg = "[Source: other] ";
			break;
		}
		default:
		{
			msg = "[Source: unknown] ";
			break;
		}
		}

		switch (type)
		{
		case AL_DEBUG_TYPE_ERROR_EXT:
		{
			msg += "[Type: error] ";
			break;
		}
		case AL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_EXT:
		{
			msg += "[Type: deprecated behavior] ";
			break;
		}
		case AL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_EXT:
		{
			msg += "[Type: undefined behavior] ";
			break;
		}
		case AL_DEBUG_TYPE_PORTABILITY_EXT:
		{
			msg += "[Type: portability] ";
			break;
		}
		case AL_DEBUG_TYPE_PERFORMANCE_EXT:
		{
			msg += "[Type: performance] ";
			break;
		}
		case AL_DEBUG_TYPE_MARKER_EXT:
		{
			msg += "[Type: custom marker] ";
			break;
		}
		case AL_DEBUG_TYPE_PUSH_GROUP_EXT:
		{
			msg += "[Type: group push] ";
			break;
		}
		case AL_DEBUG_TYPE_POP_GROUP_EXT:
		{
			msg += "[Type: group pop] ";
			break;
		}
		case AL_DEBUG_TYPE_OTHER_EXT:
		{
			msg += "[Type: other] ";
			break;
		}
		default:
		{
			msg += "[Type: unknown] ";
			break;
		}
		}

		// Send final message out
		msg += message;
		switch (severity)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case AL_DEBUG_SEVERITY_HIGH_EXT:
		{
			Logger::Error("[OpenAL HIGH] " + msg);
			break;
		}
		case AL_DEBUG_SEVERITY_MEDIUM_EXT:
		{
			Logger::Warning("[OpenAL MEDIUM] " + msg);
			break;
		}
		case AL_DEBUG_SEVERITY_LOW_EXT:
		{
			Logger::Info("[OpenAL LOW] " + msg);
			break;
		}
		case AL_DEBUG_SEVERITY_NOTIFICATION_EXT:
		{
			Logger::Info("[OpenAL NOTIFICATION] " + msg);
			break;
		}
		}
#endif
	}

	Device::~Device()
	{
		if (context)
		{
			alcMakeContextCurrent(nullptr);
			ZE_ALC_CHECK(device, "Failed to unset current AL context");
			alcDestroyContext(context);
			ZE_ALC_CHECK(device, "Failed to destroy AL context");
		}
		if (device)
		{
			[[maybe_unused]] ALCboolean status = alcCloseDevice(device);
			ZE_ASSERT(status, "Error closing AL device");
		}
	}

	Expected<Device> Device::Create(U32 sampleRate) noexcept
	{
		Device dev = {};
		dev.device = alcOpenDevice(nullptr);
		if (dev.device == nullptr)
			return std::unexpected(ZE_ALC_ERROR(ALC_INVALID_DEVICE));

		// Check for ALC extension support
#define X(ext) #ext,
		std::array<const char*, KNOWN_EXTENSION_COUNT> extensionsList = { ZE_OAL_EXT_LIST };
#undef X
		for (U16 i = 0; i < KNOWN_ALC_EXTENSION_COUNT; ++i)
			dev.supportedExtensions[i + KNOWN_AL_EXTENSION_COUNT] = alcIsExtensionPresent(dev.device, extensionsList.at(i + KNOWN_AL_EXTENSION_COUNT));

		// Check required ALC extensions
#define X(ext) + 1
		constexpr U16 REQUIRED_AL_EXTENSION_COUNT = ZE_AL_EXT_LIST_REQUIRED;
		constexpr U16 REQUIRED_ALC_EXTENSION_COUNT = ZE_ALC_EXT_LIST_REQUIRED;
#undef X
		for (U16 i = 0; i < REQUIRED_ALC_EXTENSION_COUNT; ++i)
		{
			if (!dev.supportedExtensions[i + KNOWN_AL_EXTENSION_COUNT])
			{
				ZE_LOG_RET_FAILED_EXPECT(ZE_ALC_ERROR(ALC_INVALID_ENUM), "Required ALC extension is not supported: " + std::string(extensionsList.at(i + KNOWN_AL_EXTENSION_COUNT)));
			}
		}

		// Context creation attributes
		std::vector<ALCint> attrList;
		attrList.reserve(3);
		attrList.emplace_back(ALC_SYNC);
		attrList.emplace_back(ALC_FALSE);
		if (sampleRate)
		{
			attrList.emplace_back(ALC_FREQUENCY);
			attrList.emplace_back(static_cast<ALCint>(sampleRate));
		}
		if (dev.IsExtensionSupported("ALC_SOFT_output_limiter"))
		{
			attrList.emplace_back(ALC_OUTPUT_LIMITER_SOFT);
			attrList.emplace_back(ALC_TRUE);
		}
#if _ZE_DEBUG_AUDIO_API
		if (dev.IsExtensionSupported("ALC_EXT_debug"))
		{
			attrList.emplace_back(ALC_CONTEXT_FLAGS_EXT);
			attrList.emplace_back(ALC_CONTEXT_DEBUG_BIT_EXT);
		}
		else
		{
			ZE_WARNING("ALC_EXT_debug extension is not supported, debug context will not be created");
		}
#endif
		attrList.emplace_back(0);

		dev.context = alcCreateContext(dev.device, attrList.data());
		ZE_ALC_RET_FAILED_EXPECT(dev.device);

		ALCboolean success = alcMakeContextCurrent(dev.context);
		ZE_ALC_RET_FAILED_EXPECT(dev.device);
		if (!success)
			return std::unexpected(ZE_ALC_ERROR(ALC_INVALID_CONTEXT));

		// Check AL extension support
		for (U16 i = 0; i < KNOWN_AL_EXTENSION_COUNT; ++i)
			dev.supportedExtensions[i] = alIsExtensionPresent(extensionsList.at(i));
		// Check required AL extensions
		for (U16 i = 0; i < REQUIRED_AL_EXTENSION_COUNT; ++i)
		{
			if (!dev.supportedExtensions[i])
			{
				ZE_LOG_RET_FAILED_EXPECT(ZE_AL_ERROR(AL_INVALID_ENUM), "Required AL extension is not supported: " + std::string(extensionsList.at(i)));
			}
		}
		// Must load extension functions before using any of them
		LoadExtensions(dev);

		// Setup even callback for finer streaming control
		ALenum events[] = { AL_EVENT_TYPE_BUFFER_COMPLETED_SOFT };
		alEventControlSOFT(1, events, AL_TRUE);
		ZE_AL_RET_FAILED_EXPECT();
		alEventCallbackSOFT(EventCallback, nullptr);
		ZE_AL_RET_FAILED_EXPECT();

#if _ZE_DEBUG_AUDIO_API
		if (dev.IsExtensionSupported("AL_EXT_debug"))
		{
			alDebugMessageCallbackEXT(Device::MessageCallback, nullptr);
			ZE_AL_CHECK("Failed to set AL debug message callback!");
		}
#endif

		return dev;
	}
}