#pragma once
#include "Types.h"
#include "PixelFormat.h"
#include <string>
#include <vector>
#include <deque>

namespace ZE::Utils
{
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