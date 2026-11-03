#pragma once
// Headers needed for OpenAL
#define AL_ALEXT_PROTOTYPES
#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"
#include "AL/efx.h"
#include "AL/efx-presets.h"

namespace ZE::AHI::OpenAL
{
	// Get correct volume level accepted by OpenAL
	constexpr float GetVolumeLevel(float decibels) noexcept;

#pragma region Functions
	constexpr float GetVolumeLevel(float decibels) noexcept
	{
		if (decibels < -60.0f)
			return 0.0f;
		return std::powf(10.0f, decibels / 20.0f);
	}
#pragma endregion
}