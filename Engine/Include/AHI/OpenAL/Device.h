#pragma once
#include "Error.h"
#include "OpenALExtensions.h"

namespace ZE::AHI::OpenAL
{
	class Device final
	{
		Ptr<ALCdevice> device;
		Ptr<ALCcontext> context;

#define X(ext) + 1
		static constexpr U16 KNOWN_AL_EXTENSION_COUNT = ZE_AL_EXT_LIST;
#undef X
#define X(ext) + 1
		static constexpr U16 KNOWN_ALC_EXTENSION_COUNT = ZE_ALC_EXT_LIST;
#undef X
		static constexpr U16 KNOWN_EXTENSION_COUNT = KNOWN_AL_EXTENSION_COUNT + KNOWN_ALC_EXTENSION_COUNT;

		std::bitset<KNOWN_EXTENSION_COUNT + 1> supportedExtensions;

		template<U64 SIZE>
		static constexpr U16 GetExtensionIndex(const char(&extName)[SIZE]) noexcept;

		static void MessageCallback(ALenum source, ALenum type, ALuint id, ALenum severity, ALsizei length, const ALchar* message, void* userParam) noexcept;

	public:
		Device() = default;
		ZE_CLASS_MOVE(Device);
		~Device();

		static Expected<Device> Create(U32 sampleRate) noexcept;

		Status SetVolume(float decibels) noexcept { alListenerf(AL_GAIN, GetGainLevel(decibels)); ZE_AL_RET_FAILED(); return {}; }

		// Audio API Internal

		// Only to be used on static strings
		template<U64 SIZE>
		constexpr bool IsExtensionSupported(const char(&extName)[SIZE]) const noexcept { return supportedExtensions[GetExtensionIndex<SIZE>(extName)]; }

		constexpr ALCdevice* GetDevice() const noexcept { return device; }
		constexpr ALCcontext* GetContext() const noexcept { return context; }
	};

#pragma region Functions
	template<U64 SIZE>
	constexpr U16 Device::GetExtensionIndex(const char(&extName)[SIZE]) noexcept
	{
		// __COUNTER__ is supported on MSVC, GCC and Clang, on other compilers should find alternative
		constexpr U16 COUNTER_BASE = __COUNTER__ + 1;

		switch (Utils::GetStringHash(extName, SIZE))
		{
#define X(ext) case Utils::GetStringHash(#ext, sizeof(#ext)): return __COUNTER__ - COUNTER_BASE;
			ZE_OAL_EXT_LIST
#undef X
		default:
			return KNOWN_EXTENSION_COUNT;
		}
	}
#pragma endregion
}

#if _ZE_DEBUG_AUDIO_API
// Set debug name on the AL object. Supported object types: AL_BUFFER_EXT, AL_SOURCE_EXT, AL_FILTER_EXT, AL_EFFECT_EXT, AL_AUXILIARY_EFFECT_SLOT_EXT
#	define ZE_AL_SET_DEBUG_ID(dev, objectType, object, id) do { if (dev.IsExtensionSupported("AL_EXT_debug")) { std::string __id = id; alObjectLabelEXT(objectType, object, Utils::SafeCast<ALsizei>(__id.length()), __id.c_str()); ZE_AL_CHECK("Failed to AL debug name!"); } } while(false)
#else
// Set debug name on the AL object. Supported object types: AL_BUFFER_EXT, AL_SOURCE_EXT, AL_FILTER_EXT, AL_EFFECT_EXT, AL_AUXILIARY_EFFECT_SLOT_EXT
#	define ZE_AL_SET_DEBUG_ID(dev, objectType, object, id) ((void)0)
#endif