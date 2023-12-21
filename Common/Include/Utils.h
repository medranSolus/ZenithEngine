#pragma once
#include "Types.h"
#include "PixelFormat.h"
#include <deque>
#include <limits>
#include <string>
#include <vector>

namespace ZE::Utils
{
	// Safe static cast that performs check wheter destination type can hold source value
	template<typename C, typename T>
	constexpr C SafeCast(T&& val) noexcept;

	// Encode app version in format 'major.minor.patch' into single U32
	constexpr U32 MakeVersion(U16 major, U16 minor, U16 patch) noexcept { return (static_cast<U32>(major & 0x3FF) << 22) | static_cast<U32>(minor & 0x3FF) << 12 | (patch & 0xFFF); }
	// Extract major part from encoded version
	constexpr U16 GetVersionMajor(U32 version) noexcept { return (version >> 22) & 0x3FF; }
	// Extract minor part from encoded version
	constexpr U16 GetVersionMinor(U32 version) noexcept { return (version >> 12) & 0x3FF; }
	// Extract patch part from encoded version
	constexpr U16 GetVersionPatch(U32 version) noexcept { return version & 0xFFF; }

	// Check whether formats come from same family with automatic conversion rules
	constexpr bool IsSameFormatFamily(PixelFormat f1, PixelFormat f2) noexcept;
	// Checks whether format is available for depth stencil
	constexpr bool IsDepthStencilFormat(PixelFormat format) noexcept;
	// Get string representation of PixelFormat
	constexpr const char* FormatToString(PixelFormat format) noexcept;
	// Get number of bits that pixel is occupying
	constexpr U8 GetFormatBitCount(PixelFormat format) noexcept;
	// Get bytes that single channel in pixel is occupying
	constexpr U8 GetChannelSize(PixelFormat format) noexcept;
	// Get number of channels in format
	constexpr U8 GetChannelCount(PixelFormat format) noexcept;

	// Calculace CRC32 hash over given buffer in compile time
	constexpr U32 CalculateCRC32(const char str[], U64 size) noexcept;
	// Calculace CRC64 hash over given buffer in compile time
	constexpr U64 CalculateCRC64(const char str[], U64 size) noexcept;
	// Calculace FNV hash over given buffer in compile time
	constexpr U64 CalculateFNV(const char str[], U64 size) noexcept;
	// Calculace DJB2 hash over given buffer in compile time
	constexpr U32 CalculateDJB2(const char str[], U64 size) noexcept;
	// Get compile-time hash of string literal using CRC32
	constexpr U32 GetStringHash(const char str[], U64 size) noexcept { return CalculateCRC32(str, size); }

	// Convert UTF-8 string to UTF-16
	std::wstring ToUTF16(std::string_view s) noexcept;
	// Convert UTF-16 string to UTF-8
	std::string ToUTF8(std::wstring_view s) noexcept;
	// Seperate quoted strings into vector
	std::vector<std::string> ParseQuoted(const std::string& input) noexcept;
	// Split string based on given delimeter
	std::deque<std::string_view> SplitString(std::string_view input, std::string_view delimeter) noexcept;

	// Allocate aligned memory with power of 2 alignment
	void* AlignedAlloc(U64 size, U64 alignment) noexcept;
	// Reallocate aligned memory with power of 2 alignment
	void* AlignedRealloc(void* ptr, U64 newSize, U64 oldSize, U64 alignment) noexcept;
	// Free aligned memory
	void AlignedFree(void* ptr) noexcept;

	// Returns formated timestamp of current time
	std::string GetCurrentTimestamp(bool fileFormatting = false) noexcept;

#pragma region Functions
	template<typename C, typename T>
	constexpr C SafeCast(T&& val) noexcept
	{
		ZE_WARNING_PUSH
			ZE_ASSERT(std::numeric_limits<C>::max() >= val, "Casting value that exceedes maximal size of destination type!");
		ZE_WARNING_POP
			return static_cast<C>(val);
	}

	constexpr bool IsSameFormatFamily(PixelFormat f1, PixelFormat f2) noexcept
	{
		switch (f1)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case PixelFormat::Unknown:
			return false;
		case PixelFormat::R32G32B32A32_Float:
		case PixelFormat::R32G32B32A32_UInt:
		case PixelFormat::R32G32B32A32_SInt:
		{
			switch (f2)
			{
			case PixelFormat::R32G32B32A32_Float:
			case PixelFormat::R32G32B32A32_UInt:
			case PixelFormat::R32G32B32A32_SInt:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::R16G16B16A16_Float:
		case PixelFormat::R16G16B16A16_UInt:
		case PixelFormat::R16G16B16A16_SInt:
		case PixelFormat::R16G16B16A16_UNorm:
		case PixelFormat::R16G16B16A16_SNorm:
		{
			switch (f2)
			{
			case PixelFormat::R16G16B16A16_Float:
			case PixelFormat::R16G16B16A16_UInt:
			case PixelFormat::R16G16B16A16_SInt:
			case PixelFormat::R16G16B16A16_UNorm:
			case PixelFormat::R16G16B16A16_SNorm:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::R8G8B8A8_UInt:
		case PixelFormat::R8G8B8A8_SInt:
		case PixelFormat::R8G8B8A8_UNorm:
		case PixelFormat::R8G8B8A8_SNorm:
		case PixelFormat::B8G8R8A8_UNorm:
		{
			switch (f2)
			{
			case PixelFormat::R8G8B8A8_UInt:
			case PixelFormat::R8G8B8A8_SInt:
			case PixelFormat::R8G8B8A8_UNorm:
			case PixelFormat::R8G8B8A8_SNorm:
			case PixelFormat::B8G8R8A8_UNorm:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::R8G8B8A8_UNorm_SRGB:
		case PixelFormat::B8G8R8A8_UNorm_SRGB:
		{
			switch (f2)
			{
			case PixelFormat::R8G8B8A8_UNorm_SRGB:
			case PixelFormat::B8G8R8A8_UNorm_SRGB:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::R32G32B32_Float:
		case PixelFormat::R32G32B32_UInt:
		case PixelFormat::R32G32B32_SInt:
		{
			switch (f2)
			{
			case PixelFormat::R32G32B32_Float:
			case PixelFormat::R32G32B32_UInt:
			case PixelFormat::R32G32B32_SInt:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::R32G32_Float:
		case PixelFormat::R32G32_UInt:
		case PixelFormat::R32G32_SInt:
		{
			switch (f2)
			{
			case PixelFormat::R32G32_Float:
			case PixelFormat::R32G32_UInt:
			case PixelFormat::R32G32_SInt:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::R16G16_Float:
		case PixelFormat::R16G16_UInt:
		case PixelFormat::R16G16_SInt:
		case PixelFormat::R16G16_UNorm:
		case PixelFormat::R16G16_SNorm:
		{
			switch (f2)
			{
			case PixelFormat::R16G16_Float:
			case PixelFormat::R16G16_UInt:
			case PixelFormat::R16G16_SInt:
			case PixelFormat::R16G16_UNorm:
			case PixelFormat::R16G16_SNorm:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::R8G8_UInt:
		case PixelFormat::R8G8_SInt:
		case PixelFormat::R8G8_UNorm:
		case PixelFormat::R8G8_SNorm:
		{
			switch (f2)
			{
			case PixelFormat::R8G8_UInt:
			case PixelFormat::R8G8_SInt:
			case PixelFormat::R8G8_UNorm:
			case PixelFormat::R8G8_SNorm:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::R32_Float:
		case PixelFormat::R32_Depth:
		case PixelFormat::R32_UInt:
		case PixelFormat::R32_SInt:
		{
			switch (f2)
			{
			case PixelFormat::R32_Float:
			case PixelFormat::R32_Depth:
			case PixelFormat::R32_UInt:
			case PixelFormat::R32_SInt:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::R16_Float:
		case PixelFormat::R16_UInt:
		case PixelFormat::R16_SInt:
		case PixelFormat::R16_UNorm:
		case PixelFormat::R16_SNorm:
		case PixelFormat::R16_Depth:
		{
			switch (f2)
			{
			case PixelFormat::R16_Float:
			case PixelFormat::R16_UInt:
			case PixelFormat::R16_SInt:
			case PixelFormat::R16_UNorm:
			case PixelFormat::R16_SNorm:
			case PixelFormat::R16_Depth:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::R8_UInt:
		case PixelFormat::R8_SInt:
		case PixelFormat::R8_UNorm:
		case PixelFormat::R8_SNorm:
		{
			switch (f2)
			{
			case PixelFormat::R8_UInt:
			case PixelFormat::R8_SInt:
			case PixelFormat::R8_UNorm:
			case PixelFormat::R8_SNorm:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::R24G8_DepthStencil:
			return f2 == PixelFormat::R24G8_DepthStencil;
		case PixelFormat::R32G8_DepthStencil:
			return f2 == PixelFormat::R32G8_DepthStencil;
		case PixelFormat::R10G10B10A2_UInt:
		case PixelFormat::R10G10B10A2_UNorm:
		{
			switch (f2)
			{
			case PixelFormat::R10G10B10A2_UInt:
			case PixelFormat::R10G10B10A2_UNorm:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::R11G11B10_Float:
			return f2 == PixelFormat::R11G11B10_Float;
		case PixelFormat::R9G9B9E5_SharedExp:
			return f2 == PixelFormat::R9G9B9E5_SharedExp;
		case PixelFormat::B4G4R4A4_UNorm:
			return f2 == PixelFormat::B4G4R4A4_UNorm;
		case PixelFormat::B5G5R5A1_UNorm:
			return f2 == PixelFormat::B5G5R5A1_UNorm;
		case PixelFormat::B5G6R5_UNorm:
			return f2 == PixelFormat::B5G6R5_UNorm;
		case PixelFormat::BC1_UNorm:
			return f2 == PixelFormat::BC1_UNorm;
		case PixelFormat::BC1_UNorm_SRGB:
			return f2 == PixelFormat::BC1_UNorm_SRGB;
		case PixelFormat::BC2_UNorm:
			return f2 == PixelFormat::BC2_UNorm;
		case PixelFormat::BC2_UNorm_SRGB:
			return f2 == PixelFormat::BC2_UNorm_SRGB;
		case PixelFormat::BC3_UNorm:
			return f2 == PixelFormat::BC3_UNorm;
		case PixelFormat::BC3_UNorm_SRGB:
			return f2 == PixelFormat::BC3_UNorm_SRGB;
		case PixelFormat::BC4_UNorm:
		case PixelFormat::BC4_SNorm:
		{
			switch (f2)
			{
			case PixelFormat::BC4_UNorm:
			case PixelFormat::BC4_SNorm:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::BC5_UNorm:
		case PixelFormat::BC5_SNorm:
		{
			switch (f2)
			{
			case PixelFormat::BC5_UNorm:
			case PixelFormat::BC5_SNorm:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::BC6H_UF16:
		case PixelFormat::BC6H_SF16:
		{
			switch (f2)
			{
			case PixelFormat::BC6H_UF16:
			case PixelFormat::BC6H_SF16:
				return true;
			default:
				return false;
			}
			break;
		}
		case PixelFormat::BC7_UNorm:
			return f2 == PixelFormat::BC7_UNorm;
		case PixelFormat::BC7_UNorm_SRGB:
			return f2 == PixelFormat::BC7_UNorm_SRGB;
		case PixelFormat::YUV_Y410:
			return f2 == PixelFormat::YUV_Y410;
		case PixelFormat::YUV_Y216:
			return f2 == PixelFormat::YUV_Y216;
		case PixelFormat::YUV_Y210:
			return f2 == PixelFormat::YUV_Y210;
		case PixelFormat::YUV_YUY2:
			return f2 == PixelFormat::YUV_YUY2;
		case PixelFormat::YUV_P208:
			return f2 == PixelFormat::YUV_P208;
		case PixelFormat::YUV_P016:
			return f2 == PixelFormat::YUV_P016;
		case PixelFormat::YUV_P010:
			return f2 == PixelFormat::YUV_P010;
		case PixelFormat::YUV_NV12:
			return f2 == PixelFormat::YUV_NV12;
		}
	}

	constexpr bool IsDepthStencilFormat(PixelFormat format) noexcept
	{
		switch (format)
		{
		case PixelFormat::R32_Depth:
		case PixelFormat::R16_Depth:
		case PixelFormat::R32G8_DepthStencil:
		case PixelFormat::R24G8_DepthStencil:
			return true;
		default:
			return false;
		}
	}

	constexpr const char* FormatToString(PixelFormat format) noexcept
	{
#define DECODE(pixelFormat) case PixelFormat::##pixelFormat: return #pixelFormat
		switch (format)
		{
		default:
			ZE_ENUM_UNHANDLED();
			DECODE(Unknown);
			DECODE(R32G32B32A32_Float);
			DECODE(R32G32B32A32_UInt);
			DECODE(R32G32B32A32_SInt);
			DECODE(R16G16B16A16_Float);
			DECODE(R16G16B16A16_UInt);
			DECODE(R16G16B16A16_SInt);
			DECODE(R16G16B16A16_UNorm);
			DECODE(R16G16B16A16_SNorm);
			DECODE(R8G8B8A8_UInt);
			DECODE(R8G8B8A8_SInt);
			DECODE(R8G8B8A8_UNorm);
			DECODE(R8G8B8A8_UNorm_SRGB);
			DECODE(R8G8B8A8_SNorm);
			DECODE(B8G8R8A8_UNorm);
			DECODE(B8G8R8A8_UNorm_SRGB);
			DECODE(R32G32B32_Float);
			DECODE(R32G32B32_UInt);
			DECODE(R32G32B32_SInt);
			DECODE(R32G32_Float);
			DECODE(R32G32_UInt);
			DECODE(R32G32_SInt);
			DECODE(R16G16_Float);
			DECODE(R16G16_UInt);
			DECODE(R16G16_SInt);
			DECODE(R16G16_UNorm);
			DECODE(R16G16_SNorm);
			DECODE(R8G8_UInt);
			DECODE(R8G8_SInt);
			DECODE(R8G8_UNorm);
			DECODE(R8G8_SNorm);
			DECODE(R32_Float);
			DECODE(R32_Depth);
			DECODE(R32_UInt);
			DECODE(R32_SInt);
			DECODE(R16_Float);
			DECODE(R16_UInt);
			DECODE(R16_SInt);
			DECODE(R16_UNorm);
			DECODE(R16_SNorm);
			DECODE(R16_Depth);
			DECODE(R8_UInt);
			DECODE(R8_SInt);
			DECODE(R8_UNorm);
			DECODE(R8_SNorm);
			DECODE(R24G8_DepthStencil);
			DECODE(R32G8_DepthStencil);
			DECODE(R10G10B10A2_UInt);
			DECODE(R10G10B10A2_UNorm);
			DECODE(R11G11B10_Float);
			DECODE(R9G9B9E5_SharedExp);
			DECODE(B4G4R4A4_UNorm);
			DECODE(B5G5R5A1_UNorm);
			DECODE(B5G6R5_UNorm);
			DECODE(BC1_UNorm);
			DECODE(BC1_UNorm_SRGB);
			DECODE(BC2_UNorm);
			DECODE(BC2_UNorm_SRGB);
			DECODE(BC3_UNorm);
			DECODE(BC3_UNorm_SRGB);
			DECODE(BC4_UNorm);
			DECODE(BC4_SNorm);
			DECODE(BC5_UNorm);
			DECODE(BC5_SNorm);
			DECODE(BC6H_UF16);
			DECODE(BC6H_SF16);
			DECODE(BC7_UNorm);
			DECODE(BC7_UNorm_SRGB);
			DECODE(YUV_Y410);
			DECODE(YUV_Y216);
			DECODE(YUV_Y210);
			DECODE(YUV_YUY2);
			DECODE(YUV_P208);
			DECODE(YUV_P016);
			DECODE(YUV_P010);
			DECODE(YUV_NV12);
		}
#undef DECODE
	}

	constexpr U8 GetFormatBitCount(PixelFormat format) noexcept
	{
		switch (format)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case PixelFormat::Unknown:
			return 0;
		case PixelFormat::R32G32B32A32_Float:
		case PixelFormat::R32G32B32A32_UInt:
		case PixelFormat::R32G32B32A32_SInt:
		case PixelFormat::BC2_UNorm:
		case PixelFormat::BC2_UNorm_SRGB:
		case PixelFormat::BC3_UNorm:
		case PixelFormat::BC3_UNorm_SRGB:
		case PixelFormat::BC5_UNorm:
		case PixelFormat::BC5_SNorm:
		case PixelFormat::BC6H_UF16:
		case PixelFormat::BC6H_SF16:
		case PixelFormat::BC7_UNorm:
		case PixelFormat::BC7_UNorm_SRGB:
			return 128;
		case PixelFormat::R32G32B32_Float:
		case PixelFormat::R32G32B32_UInt:
		case PixelFormat::R32G32B32_SInt:
			return 96;
		case PixelFormat::R16G16B16A16_Float:
		case PixelFormat::R16G16B16A16_UInt:
		case PixelFormat::R16G16B16A16_SInt:
		case PixelFormat::R16G16B16A16_UNorm:
		case PixelFormat::R16G16B16A16_SNorm:
		case PixelFormat::R32G32_Float:
		case PixelFormat::R32G32_UInt:
		case PixelFormat::R32G32_SInt:
		case PixelFormat::R32G8_DepthStencil: // In fact second component of depth stencil takes 32 bits of real memory
		case PixelFormat::YUV_Y216:
		case PixelFormat::YUV_Y210:
			return 64;
		case PixelFormat::R8G8B8A8_UInt:
		case PixelFormat::R8G8B8A8_SInt:
		case PixelFormat::R8G8B8A8_UNorm:
		case PixelFormat::R8G8B8A8_UNorm_SRGB:
		case PixelFormat::R8G8B8A8_SNorm:
		case PixelFormat::B8G8R8A8_UNorm:
		case PixelFormat::B8G8R8A8_UNorm_SRGB:
		case PixelFormat::R16G16_Float:
		case PixelFormat::R16G16_UInt:
		case PixelFormat::R16G16_SInt:
		case PixelFormat::R16G16_UNorm:
		case PixelFormat::R16G16_SNorm:
		case PixelFormat::R32_Float:
		case PixelFormat::R32_Depth:
		case PixelFormat::R32_UInt:
		case PixelFormat::R32_SInt:
		case PixelFormat::R24G8_DepthStencil:
		case PixelFormat::R10G10B10A2_UInt:
		case PixelFormat::R10G10B10A2_UNorm:
		case PixelFormat::R11G11B10_Float:
		case PixelFormat::R9G9B9E5_SharedExp:
		case PixelFormat::YUV_Y410:
		case PixelFormat::YUV_YUY2:
			return 32;
		case PixelFormat::R8G8_UInt:
		case PixelFormat::R8G8_SInt:
		case PixelFormat::R8G8_UNorm:
		case PixelFormat::R8G8_SNorm:
		case PixelFormat::R16_Float:
		case PixelFormat::R16_UInt:
		case PixelFormat::R16_SInt:
		case PixelFormat::R16_UNorm:
		case PixelFormat::R16_SNorm:
		case PixelFormat::R16_Depth:
		case PixelFormat::B4G4R4A4_UNorm:
		case PixelFormat::B5G5R5A1_UNorm:
		case PixelFormat::B5G6R5_UNorm:
		case PixelFormat::YUV_P016:
		case PixelFormat::YUV_P010:
			return 16;
		case PixelFormat::R8_UInt:
		case PixelFormat::R8_SInt:
		case PixelFormat::R8_UNorm:
		case PixelFormat::R8_SNorm:
		case PixelFormat::YUV_P208:
		case PixelFormat::YUV_NV12:
			return 8;
		case PixelFormat::BC1_UNorm:
		case PixelFormat::BC1_UNorm_SRGB:
		case PixelFormat::BC4_UNorm:
		case PixelFormat::BC4_SNorm:
			return 4;
		}
	}

	constexpr U8 GetChannelSize(PixelFormat format) noexcept
	{
		switch (format)
		{
		case PixelFormat::R32G32B32A32_Float:
		case PixelFormat::R32G32B32A32_UInt:
		case PixelFormat::R32G32B32A32_SInt:
		case PixelFormat::R32G32B32_Float:
		case PixelFormat::R32G32B32_UInt:
		case PixelFormat::R32G32B32_SInt:
		case PixelFormat::R32G32_Float:
		case PixelFormat::R32G32_UInt:
		case PixelFormat::R32G32_SInt:
		case PixelFormat::R32_Float:
		case PixelFormat::R32_Depth:
		case PixelFormat::R32_UInt:
		case PixelFormat::R32_SInt:
			return 4;
		case PixelFormat::R16G16B16A16_Float:
		case PixelFormat::R16G16B16A16_UInt:
		case PixelFormat::R16G16B16A16_SInt:
		case PixelFormat::R16G16B16A16_UNorm:
		case PixelFormat::R16G16B16A16_SNorm:
		case PixelFormat::R16G16_Float:
		case PixelFormat::R16G16_UInt:
		case PixelFormat::R16G16_SInt:
		case PixelFormat::R16G16_UNorm:
		case PixelFormat::R16G16_SNorm:
		case PixelFormat::R16_Float:
		case PixelFormat::R16_UInt:
		case PixelFormat::R16_SInt:
		case PixelFormat::R16_UNorm:
		case PixelFormat::R16_SNorm:
		case PixelFormat::R16_Depth:
			return 2;
		case PixelFormat::R8G8B8A8_UInt:
		case PixelFormat::R8G8B8A8_SInt:
		case PixelFormat::R8G8B8A8_UNorm:
		case PixelFormat::R8G8B8A8_UNorm_SRGB:
		case PixelFormat::R8G8B8A8_SNorm:
		case PixelFormat::B8G8R8A8_UNorm:
		case PixelFormat::B8G8R8A8_UNorm_SRGB:
		case PixelFormat::R8G8_UInt:
		case PixelFormat::R8G8_SInt:
		case PixelFormat::R8G8_UNorm:
		case PixelFormat::R8G8_SNorm:
		case PixelFormat::R8_UInt:
		case PixelFormat::R8_SInt:
		case PixelFormat::R8_UNorm:
		case PixelFormat::R8_SNorm:
			return 1;
			// Non-standard formats that require special handling
		default:
			ZE_ENUM_UNHANDLED();
		case PixelFormat::Unknown:
		case PixelFormat::R24G8_DepthStencil:
		case PixelFormat::R32G8_DepthStencil:
		case PixelFormat::R10G10B10A2_UInt:
		case PixelFormat::R10G10B10A2_UNorm:
		case PixelFormat::R11G11B10_Float:
		case PixelFormat::R9G9B9E5_SharedExp:
		case PixelFormat::B4G4R4A4_UNorm:
		case PixelFormat::B5G5R5A1_UNorm:
		case PixelFormat::B5G6R5_UNorm:
		case PixelFormat::BC1_UNorm:
		case PixelFormat::BC1_UNorm_SRGB:
		case PixelFormat::BC2_UNorm:
		case PixelFormat::BC2_UNorm_SRGB:
		case PixelFormat::BC3_UNorm:
		case PixelFormat::BC3_UNorm_SRGB:
		case PixelFormat::BC4_UNorm:
		case PixelFormat::BC4_SNorm:
		case PixelFormat::BC5_UNorm:
		case PixelFormat::BC5_SNorm:
		case PixelFormat::BC6H_UF16:
		case PixelFormat::BC6H_SF16:
		case PixelFormat::BC7_UNorm:
		case PixelFormat::BC7_UNorm_SRGB:
		case PixelFormat::YUV_Y410:
		case PixelFormat::YUV_Y216:
		case PixelFormat::YUV_Y210:
		case PixelFormat::YUV_YUY2:
		case PixelFormat::YUV_P208:
		case PixelFormat::YUV_P016:
		case PixelFormat::YUV_P010:
		case PixelFormat::YUV_NV12:
			return 0;
		}
	}

	constexpr U8 GetChannelCount(PixelFormat format) noexcept
	{
		switch (format)
		{
		case PixelFormat::R32G32B32A32_Float:
		case PixelFormat::R32G32B32A32_UInt:
		case PixelFormat::R32G32B32A32_SInt:
		case PixelFormat::R16G16B16A16_Float:
		case PixelFormat::R16G16B16A16_UInt:
		case PixelFormat::R16G16B16A16_SInt:
		case PixelFormat::R16G16B16A16_UNorm:
		case PixelFormat::R16G16B16A16_SNorm:
		case PixelFormat::R8G8B8A8_UInt:
		case PixelFormat::R8G8B8A8_SInt:
		case PixelFormat::R8G8B8A8_UNorm:
		case PixelFormat::R8G8B8A8_UNorm_SRGB:
		case PixelFormat::R8G8B8A8_SNorm:
		case PixelFormat::B8G8R8A8_UNorm:
		case PixelFormat::B8G8R8A8_UNorm_SRGB:
		case PixelFormat::R10G10B10A2_UInt:
		case PixelFormat::R10G10B10A2_UNorm:
		case PixelFormat::R9G9B9E5_SharedExp:
		case PixelFormat::B4G4R4A4_UNorm:
		case PixelFormat::B5G5R5A1_UNorm:
			return 4;
		case PixelFormat::R32G32B32_Float:
		case PixelFormat::R32G32B32_UInt:
		case PixelFormat::R32G32B32_SInt:
		case PixelFormat::R11G11B10_Float:
		case PixelFormat::B5G6R5_UNorm:
			return 3;
		case PixelFormat::R32G32_Float:
		case PixelFormat::R32G32_UInt:
		case PixelFormat::R32G32_SInt:
		case PixelFormat::R16G16_Float:
		case PixelFormat::R16G16_UInt:
		case PixelFormat::R16G16_SInt:
		case PixelFormat::R16G16_UNorm:
		case PixelFormat::R16G16_SNorm:
		case PixelFormat::R8G8_UInt:
		case PixelFormat::R8G8_SInt:
		case PixelFormat::R8G8_UNorm:
		case PixelFormat::R8G8_SNorm:
		case PixelFormat::R24G8_DepthStencil:
		case PixelFormat::R32G8_DepthStencil:
			return 2;
		case PixelFormat::R32_Float:
		case PixelFormat::R32_Depth:
		case PixelFormat::R32_UInt:
		case PixelFormat::R32_SInt:
		case PixelFormat::R16_Float:
		case PixelFormat::R16_UInt:
		case PixelFormat::R16_SInt:
		case PixelFormat::R16_UNorm:
		case PixelFormat::R16_SNorm:
		case PixelFormat::R16_Depth:
		case PixelFormat::R8_UInt:
		case PixelFormat::R8_SInt:
		case PixelFormat::R8_UNorm:
		case PixelFormat::R8_SNorm:
			return 1;
			// Non-standard formats that require special handling
		default:
			ZE_ENUM_UNHANDLED();
		case PixelFormat::Unknown:
		case PixelFormat::BC1_UNorm:
		case PixelFormat::BC1_UNorm_SRGB:
		case PixelFormat::BC2_UNorm:
		case PixelFormat::BC2_UNorm_SRGB:
		case PixelFormat::BC3_UNorm:
		case PixelFormat::BC3_UNorm_SRGB:
		case PixelFormat::BC4_UNorm:
		case PixelFormat::BC4_SNorm:
		case PixelFormat::BC5_UNorm:
		case PixelFormat::BC5_SNorm:
		case PixelFormat::BC6H_UF16:
		case PixelFormat::BC6H_SF16:
		case PixelFormat::BC7_UNorm:
		case PixelFormat::BC7_UNorm_SRGB:
		case PixelFormat::YUV_Y410:
		case PixelFormat::YUV_Y216:
		case PixelFormat::YUV_Y210:
		case PixelFormat::YUV_YUY2:
		case PixelFormat::YUV_P208:
		case PixelFormat::YUV_P016:
		case PixelFormat::YUV_P010:
		case PixelFormat::YUV_NV12:
			return 0;
		}
	}

	constexpr U32 CalculateCRC32(const char str[], U64 size) noexcept
	{
		constexpr U32 lookupTable[256]
		{
			 0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
			 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
			 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
			 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
			 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
			 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
			 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
			 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
			 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
			 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
			 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
			 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
			 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
			 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
			 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
			 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
			 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
			 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
			 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
			 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
			 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
			 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
			 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
			 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
			 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
			 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
			 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
			 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
			 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
			 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
			 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
			 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
		};

		U32 crc = UINT32_MAX;
		for (U64 i = 0; i < size; ++i)
			crc = (crc >> 8) ^ lookupTable[(crc ^ str[i]) & 0xFF];
		return crc ^ UINT32_MAX;
	}

	constexpr U64 CalculateCRC64(const char str[], U64 size) noexcept
	{
		constexpr U64 lookupTable[256]
		{
			0x0000000000000000, 0x42F0E1EBA9EA3693, 0x85E1C3D753D46D26, 0xC711223CFA3E5BB5, 0x493366450E42ECDF, 0x0BC387AEA7A8DA4C, 0xCCD2A5925D9681F9, 0x8E224479F47CB76A,
			0x9266CC8A1C85D9BE, 0xD0962D61B56FEF2D, 0x17870F5D4F51B498, 0x5577EEB6E6BB820B, 0xDB55AACF12C73561, 0x99A54B24BB2D03F2, 0x5EB4691841135847, 0x1C4488F3E8F96ED4,
			0x663D78FF90E185EF, 0x24CD9914390BB37C, 0xE3DCBB28C335E8C9, 0xA12C5AC36ADFDE5A, 0x2F0E1EBA9EA36930, 0x6DFEFF5137495FA3, 0xAAEFDD6DCD770416, 0xE81F3C86649D3285,
			0xF45BB4758C645C51, 0xB6AB559E258E6AC2, 0x71BA77A2DFB03177, 0x334A9649765A07E4, 0xBD68D2308226B08E, 0xFF9833DB2BCC861D, 0x388911E7D1F2DDA8, 0x7A79F00C7818EB3B,
			0xCC7AF1FF21C30BDE, 0x8E8A101488293D4D, 0x499B3228721766F8, 0x0B6BD3C3DBFD506B, 0x854997BA2F81E701, 0xC7B97651866BD192, 0x00A8546D7C558A27, 0x4258B586D5BFBCB4,
			0x5E1C3D753D46D260, 0x1CECDC9E94ACE4F3, 0xDBFDFEA26E92BF46, 0x990D1F49C77889D5, 0x172F5B3033043EBF, 0x55DFBADB9AEE082C, 0x92CE98E760D05399, 0xD03E790CC93A650A,
			0xAA478900B1228E31, 0xE8B768EB18C8B8A2, 0x2FA64AD7E2F6E317, 0x6D56AB3C4B1CD584, 0xE374EF45BF6062EE, 0xA1840EAE168A547D, 0x66952C92ECB40FC8, 0x2465CD79455E395B,
			0x3821458AADA7578F, 0x7AD1A461044D611C, 0xBDC0865DFE733AA9, 0xFF3067B657990C3A, 0x711223CFA3E5BB50, 0x33E2C2240A0F8DC3, 0xF4F3E018F031D676, 0xB60301F359DBE0E5,
			0xDA050215EA6C212F, 0x98F5E3FE438617BC, 0x5FE4C1C2B9B84C09, 0x1D14202910527A9A, 0x93366450E42ECDF0, 0xD1C685BB4DC4FB63, 0x16D7A787B7FAA0D6, 0x5427466C1E109645,
			0x4863CE9FF6E9F891, 0x0A932F745F03CE02, 0xCD820D48A53D95B7, 0x8F72ECA30CD7A324, 0x0150A8DAF8AB144E, 0x43A04931514122DD, 0x84B16B0DAB7F7968, 0xC6418AE602954FFB,
			0xBC387AEA7A8DA4C0, 0xFEC89B01D3679253, 0x39D9B93D2959C9E6, 0x7B2958D680B3FF75, 0xF50B1CAF74CF481F, 0xB7FBFD44DD257E8C, 0x70EADF78271B2539, 0x321A3E938EF113AA,
			0x2E5EB66066087D7E, 0x6CAE578BCFE24BED, 0xABBF75B735DC1058, 0xE94F945C9C3626CB, 0x676DD025684A91A1, 0x259D31CEC1A0A732, 0xE28C13F23B9EFC87, 0xA07CF2199274CA14,
			0x167FF3EACBAF2AF1, 0x548F120162451C62, 0x939E303D987B47D7, 0xD16ED1D631917144, 0x5F4C95AFC5EDC62E, 0x1DBC74446C07F0BD, 0xDAAD56789639AB08, 0x985DB7933FD39D9B,
			0x84193F60D72AF34F, 0xC6E9DE8B7EC0C5DC, 0x01F8FCB784FE9E69, 0x43081D5C2D14A8FA, 0xCD2A5925D9681F90, 0x8FDAB8CE70822903, 0x48CB9AF28ABC72B6, 0x0A3B7B1923564425,
			0x70428B155B4EAF1E, 0x32B26AFEF2A4998D, 0xF5A348C2089AC238, 0xB753A929A170F4AB, 0x3971ED50550C43C1, 0x7B810CBBFCE67552, 0xBC902E8706D82EE7, 0xFE60CF6CAF321874,
			0xE224479F47CB76A0, 0xA0D4A674EE214033, 0x67C58448141F1B86, 0x253565A3BDF52D15, 0xAB1721DA49899A7F, 0xE9E7C031E063ACEC, 0x2EF6E20D1A5DF759, 0x6C0603E6B3B7C1CA,
			0xF6FAE5C07D3274CD, 0xB40A042BD4D8425E, 0x731B26172EE619EB, 0x31EBC7FC870C2F78, 0xBFC9838573709812, 0xFD39626EDA9AAE81, 0x3A28405220A4F534, 0x78D8A1B9894EC3A7,
			0x649C294A61B7AD73, 0x266CC8A1C85D9BE0, 0xE17DEA9D3263C055, 0xA38D0B769B89F6C6, 0x2DAF4F0F6FF541AC, 0x6F5FAEE4C61F773F, 0xA84E8CD83C212C8A, 0xEABE6D3395CB1A19,
			0x90C79D3FEDD3F122, 0xD2377CD44439C7B1, 0x15265EE8BE079C04, 0x57D6BF0317EDAA97, 0xD9F4FB7AE3911DFD, 0x9B041A914A7B2B6E, 0x5C1538ADB04570DB, 0x1EE5D94619AF4648,
			0x02A151B5F156289C, 0x4051B05E58BC1E0F, 0x87409262A28245BA, 0xC5B073890B687329, 0x4B9237F0FF14C443, 0x0962D61B56FEF2D0, 0xCE73F427ACC0A965, 0x8C8315CC052A9FF6,
			0x3A80143F5CF17F13, 0x7870F5D4F51B4980, 0xBF61D7E80F251235, 0xFD913603A6CF24A6, 0x73B3727A52B393CC, 0x31439391FB59A55F, 0xF652B1AD0167FEEA, 0xB4A25046A88DC879,
			0xA8E6D8B54074A6AD, 0xEA16395EE99E903E, 0x2D071B6213A0CB8B, 0x6FF7FA89BA4AFD18, 0xE1D5BEF04E364A72, 0xA3255F1BE7DC7CE1, 0x64347D271DE22754, 0x26C49CCCB40811C7,
			0x5CBD6CC0CC10FAFC, 0x1E4D8D2B65FACC6F, 0xD95CAF179FC497DA, 0x9BAC4EFC362EA149, 0x158E0A85C2521623, 0x577EEB6E6BB820B0, 0x906FC95291867B05, 0xD29F28B9386C4D96,
			0xCEDBA04AD0952342, 0x8C2B41A1797F15D1, 0x4B3A639D83414E64, 0x09CA82762AAB78F7, 0x87E8C60FDED7CF9D, 0xC51827E4773DF90E, 0x020905D88D03A2BB, 0x40F9E43324E99428,
			0x2CFFE7D5975E55E2, 0x6E0F063E3EB46371, 0xA91E2402C48A38C4, 0xEBEEC5E96D600E57, 0x65CC8190991CB93D, 0x273C607B30F68FAE, 0xE02D4247CAC8D41B, 0xA2DDA3AC6322E288,
			0xBE992B5F8BDB8C5C, 0xFC69CAB42231BACF, 0x3B78E888D80FE17A, 0x7988096371E5D7E9, 0xF7AA4D1A85996083, 0xB55AACF12C735610, 0x724B8ECDD64D0DA5, 0x30BB6F267FA73B36,
			0x4AC29F2A07BFD00D, 0x08327EC1AE55E69E, 0xCF235CFD546BBD2B, 0x8DD3BD16FD818BB8, 0x03F1F96F09FD3CD2, 0x41011884A0170A41, 0x86103AB85A2951F4, 0xC4E0DB53F3C36767,
			0xD8A453A01B3A09B3, 0x9A54B24BB2D03F20, 0x5D45907748EE6495, 0x1FB5719CE1045206, 0x919735E51578E56C, 0xD367D40EBC92D3FF, 0x1476F63246AC884A, 0x568617D9EF46BED9,
			0xE085162AB69D5E3C, 0xA275F7C11F7768AF, 0x6564D5FDE549331A, 0x279434164CA30589, 0xA9B6706FB8DFB2E3, 0xEB46918411358470, 0x2C57B3B8EB0BDFC5, 0x6EA7525342E1E956,
			0x72E3DAA0AA188782, 0x30133B4B03F2B111, 0xF7021977F9CCEAA4, 0xB5F2F89C5026DC37, 0x3BD0BCE5A45A6B5D, 0x79205D0E0DB05DCE, 0xBE317F32F78E067B, 0xFCC19ED95E6430E8,
			0x86B86ED5267CDBD3, 0xC4488F3E8F96ED40, 0x0359AD0275A8B6F5, 0x41A94CE9DC428066, 0xCF8B0890283E370C, 0x8D7BE97B81D4019F, 0x4A6ACB477BEA5A2A, 0x089A2AACD2006CB9,
			0x14DEA25F3AF9026D, 0x562E43B4931334FE, 0x913F6188692D6F4B, 0xD3CF8063C0C759D8, 0x5DEDC41A34BBEEB2, 0x1F1D25F19D51D821, 0xD80C07CD676F8394, 0x9AFCE626CE85B507
		};

		U64 crc = UINT64_MAX;
		for (U64 i = 0; i < size; ++i)
			crc = (crc << 8) ^ lookupTable[((crc >> 56) ^ str[i]) & 0xFF];
		return crc ^ UINT64_MAX;
	}

	constexpr U64 CalculateFNV(const char str[], U64 size) noexcept
	{
		U64 value = 0xCBF29CE484222325;
		for (U64 i = 0; i < size; ++i)
			value = (value ^ str[i]) * 1099511628211;
		return value;
	}

	constexpr U32 CalculateDJB2(const char str[], U64 size) noexcept
	{
		U32 hash = 5381;
		for (U64 i = 0; i < size; ++i)
			hash = 33 * hash + str[i];
		return hash;
	}
#pragma endregion
}