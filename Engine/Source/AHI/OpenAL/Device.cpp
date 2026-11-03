#include "AHI/OpenAL/Device.h"

namespace ZE::AHI::OpenAL
{
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
			ZE_ALC_CHECK(device, "Failed to close AL device");
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
		if constexpr (_ZE_DEBUG_AUDIO_API)
		{
			if (dev.supportedExtensions[GetExtensionIndex("ALC_EXT_debug")])
			{
				attrList.emplace_back(ALC_CONTEXT_FLAGS_EXT);
				attrList.emplace_back(ALC_CONTEXT_DEBUG_BIT_EXT);
			}
			else
			{
				ZE_WARNING("ALC_EXT_debug extension is not supported, debug context will not be created");
			}
		}
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

		return dev;
	}
}