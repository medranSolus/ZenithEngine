#pragma once
#include "SFX/ChannelMask.h"
// Headers needed for OpenAL
#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"
#include "AL/efx.h"
#include "AL/efx-presets.h"

namespace ZE::AHI::OpenAL
{
	// Get correct volume level accepted by OpenAL
	constexpr float GetVolumeLevel(float decibels) noexcept;

	// Load supported OpenAL extension's function pointers
	void LoadExtensions(class Device& dev) noexcept;
	// Get OpenAL format description of sound data
	ALenum GetALFormat(class Device& dev, SFX::ChannelMask channels, U8 bitsPerSample, bool isFloat) noexcept;

#pragma region Functions
	constexpr float GetVolumeLevel(float decibels) noexcept
	{
		if (decibels < -60.0f)
			return 0.0f;
		return std::powf(10.0f, decibels / 20.0f);
	}
#pragma endregion

	// Extensions function pointers loaded at runtime (only used ones)
#pragma region Extension functions
	extern LPALDEBUGMESSAGECALLBACKEXT alDebugMessageCallbackEXT;
	extern LPALOBJECTLABELEXT alObjectLabelEXT;
	extern PFNALBUFFERDATASTATICPROC alBufferDataStatic;
#pragma endregion
}