#pragma once
#include "Types.h"
#include "PixelFormat.h"
#include <string>
#include <vector>
#include <deque>

namespace ZE::Utils
{
	// Encode app version in format 'major.minor.patch' into single uint32_t
	constexpr uint32_t MakeVersion(uint16_t major, uint16_t minor, uint16_t patch) noexcept { return (static_cast<uint32_t>(major & 0x3FF) << 22) | static_cast<uint32_t>(minor & 0x3FF) << 12 | (patch & 0xFFF); }
	constexpr uint64_t GetVersionMajor(uint32_t version) noexcept { return (version >> 22) & 0x3FF; }
	constexpr uint64_t GetVersionMinor(uint32_t version) noexcept { return (version >> 12) & 0x3FF; }
	constexpr uint64_t GetVersionPatch(uint32_t version) noexcept { return version & 0xFFF; }

	// Checks wheter format is avaiable for depth stencil
	constexpr bool IsDepthStencilFormat(PixelFormat format) noexcept;

	std::wstring ToUtf8(const std::string& s) noexcept;
	std::string ToAscii(const std::wstring& s) noexcept;
	std::vector<std::string> ParseQuoted(const std::string& input) noexcept;
	std::deque<std::string> SplitString(const std::string& input, const std::string& delimeter) noexcept;

#pragma region Functions
	constexpr bool IsDepthStencilFormat(PixelFormat format) noexcept
	{
		switch (format)
		{
		case ZE::PixelFormat::R32_Typeless:
		case ZE::PixelFormat::R32_Depth:
		case ZE::PixelFormat::R16_Depth:
		case ZE::PixelFormat::R24G8_Typeless:
		case ZE::PixelFormat::R24_UNorm_X8_Typeless:
		case ZE::PixelFormat::R24_Depth_S8_UInt:
		case ZE::PixelFormat::X24_Typeless_G8_UInt:
		case ZE::PixelFormat::R32_Depth_S8X24_UInt:
		case ZE::PixelFormat::R32_Float_X8X24_Typeless:
		case ZE::PixelFormat::X32_Typeless_G8X24_UInt:
			return true;
		default:
			return false;
		}
	}
#pragma endregion
}