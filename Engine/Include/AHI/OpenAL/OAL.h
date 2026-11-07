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
	// Get correct volume gain level accepted by OpenAL
	constexpr float GetGainLevel(float decibels) noexcept;
	// Get correct pitch level accepted by OpenAL
	constexpr float GetPitchLevel(float semitones) noexcept;

	// Load supported OpenAL extension's function pointers
	void LoadExtensions(class Device& dev) noexcept;
	// Get OpenAL format description of sound data
	ALenum GetALFormat(class Device& dev, SFX::ChannelMask channels, U8 bitsPerSample, bool isFloat) noexcept;
	// Setup source parameters to disable spatialization
	Status SetSource2D(class Device& dev, ALuint source) noexcept;
	// Resume playback of a source if is paused
	Status ResumePlayback(ALuint source) noexcept;

	// Global event callback for OpenAL SOFT events extension
	void EventCallback(ALenum eventType, ALuint object, ALuint param,
		ALsizei length, const ALchar* message, void* userParam) noexcept;

#pragma region Functions
	constexpr float GetGainLevel(float decibels) noexcept
	{
		if (decibels < -60.0f)
			return 0.0f;
		return std::powf(10.0f, decibels / 20.0f);
	}

	constexpr float GetPitchLevel(float semitones) noexcept
	{
		return Math::Clamp(std::powf(2.0f, semitones / 12.0f), 0.5f, 2.0f);
	}
#pragma endregion

	// Extensions function pointers loaded at runtime (only used ones)
#pragma region Extension functions
	extern LPALDEBUGMESSAGECALLBACKEXT alDebugMessageCallbackEXT;
	extern LPALOBJECTLABELEXT alObjectLabelEXT;
	extern PFNALBUFFERDATASTATICPROC alBufferDataStatic;
	extern LPALEVENTCONTROLSOFT alEventControlSOFT;
	extern LPALEVENTCALLBACKSOFT alEventCallbackSOFT;
#pragma endregion
}