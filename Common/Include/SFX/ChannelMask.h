#pragma once
#include "Macros.h"

namespace ZE::SFX
{
	// Audio channels configuration
	typedef U32 ChannelMask;
	// Possible channel positions in a sound buffer
	enum class SoundChannel : ChannelMask
	{
		FrontLeft = 0x0001,
		FrontRight = 0x0002,
		FrontCenter = 0x0004,
		LowFrequency = 0x0008,
		BackLeft = 0x0010,
		BackRight = 0x0020,
		FrontLeftOfCenter = 0x0040,
		FrontRightOfCenter = 0x0080,
		BackCenter = 0x0100,
		SideLeft = 0x0200,
		SideRight = 0x0400,
		TopCenter = 0x0800,
		TopFrontLeft = 0x1000,
		TopFrontCenter = 0x2000,
		TopFrontRight = 0x4000,
		TopBackLeft = 0x8000,
		TopBackCenter = 0x10000,
		TopBackRight = 0x20000,

		// Shorthands for typical configurations
		Mono = FrontCenter,
		Stereo_2_0 = FrontLeft | FrontRight,
		Stereo_2_1 = FrontLeft | FrontRight | LowFrequency,
		Stereo_3_0 = FrontLeft | FrontRight | FrontCenter,
		Stereo_4_0 = FrontLeft | FrontRight | BackLeft | BackRight,
		Stereo_5_0 = FrontLeft | FrontRight | FrontCenter | BackLeft | BackRight,
		Stereo_5_1 = FrontLeft | FrontRight | FrontCenter | LowFrequency | BackLeft | BackRight,
		Stereo_6_1 = FrontLeft | FrontRight | FrontCenter | LowFrequency | BackLeft | BackRight | BackCenter,
		Stereo_7_1 = FrontLeft | FrontRight | FrontCenter | LowFrequency | BackLeft | BackRight | SideLeft | SideRight,
	};
	ZE_ENUM_OPERATORS(SoundChannel, ChannelMask);

	constexpr ChannelMask GetDefaultMask(U8 channelCount) noexcept;

#pragma region Functions
	constexpr ChannelMask GetDefaultMask(U8 channelCount) noexcept
	{
		switch (channelCount)
		{
		case 1:
			return Base(SoundChannel::Mono);
		case 2:
			return Base(SoundChannel::Stereo_2_0);
		case 3:
			return Base(SoundChannel::Stereo_3_0);
		case 4:
			return Base(SoundChannel::Stereo_4_0);
		case 5:
			return Base(SoundChannel::Stereo_5_0);
		case 6:
			return Base(SoundChannel::Stereo_5_1);
		case 7:
			return Base(SoundChannel::Stereo_6_1);
		case 8:
			return Base(SoundChannel::Stereo_7_1);
		default:
			ZE_FAIL("Unsupported number of sound channels!");
			return 0;
		}
	}
#pragma endregion
}