#pragma once
#include "Header.h"
#include "HeaderDXT10.h"
#include "PixelFormat.h"

namespace ZE::DDS
{
	// Identifier of DDS file 'DDS '
	inline constexpr U32 MAGIC_NUMBER = 0x20534444;

	// Result of DDS file operations
	enum class FileResult : U8
	{
		Ok, ReadError, IncorrectMagicNumber, UnknownFormat, MissingCubemapFaces, IllformattedVolumeTexture,
		IncorrectArraySize, Incorrect1DTextureHeight, IncorrectDimension, WriteError
	};

	// Data about corresponding surface in memory
	struct SurfaceData
	{
		PixelFormat Format;
		bool Alpha;
		U32 Width;
		U32 Height;
		U16 Depth;
		U16 MipCount;
		U16 ArraySize;
		U32 RowSize;
		U64 SliceSize;
		std::shared_ptr<U8[]> ImageMemory;
	};

	// Data retrieved from DDS file
	struct FileData
	{
		PixelFormat Format;
		bool Alpha;
		U32 Width;
		U32 Height;
		U16 Depth;
		U16 MipCount;
		U16 ArraySize;
		U32 ImageMemorySize;
		std::shared_ptr<U8[]> ImageMemory;
	};

	// Convert PixelFormat to FormatDDS
	constexpr FormatDDS GetDDSFormat(PixelFormat format) noexcept;
	// Convert FormatDDS to PixelFormat
	constexpr PixelFormat GetFormatFromDDS(FormatDDS ddsFormat) noexcept;

	// Save DDS file to disk
	FileResult EncodeFile(FILE* file, const SurfaceData& srcData) noexcept;
	// Load and parse DDS file from disk
	FileResult ParseFile(FILE* file, FileData& destData, U32 destRowAlignment, U32 destSliceAlignment) noexcept;

#pragma region Functions
	// List of mappings between PixelFormat and DDS::FormatDDS for enum decoding in X() macro
#define ZE_DDS_FORMAT_MAPPINGS \
	X(Unknown,             Unknown) \
	X(R32G32B32A32_Float,  R32G32B32A32_Float) \
	X(R32G32B32A32_UInt,   R32G32B32A32_UInt) \
	X(R32G32B32A32_SInt,   R32G32B32A32_SInt) \
	X(R32G32B32_Float,     R32G32B32_Float) \
	X(R32G32B32_UInt,      R32G32B32_UInt) \
	X(R32G32B32_SInt,      R32G32B32_SInt) \
	X(R16G16B16A16_Float,  R16G16B16A16_Float) \
	X(R16G16B16A16_UNorm,  R16G16B16A16_UNorm) \
	X(R16G16B16A16_UInt,   R16G16B16A16_UInt) \
	X(R16G16B16A16_SNorm,  R16G16B16A16_SNorm) \
	X(R16G16B16A16_SInt,   R16G16B16A16_SInt) \
	X(R32G32_Float,        R32G32_Float) \
	X(R32G32_UInt,         R32G32_UInt) \
	X(R32G32_SInt,         R32G32_SInt) \
	X(R10G10B10A2_UNorm,   R10G10B10A2_UNorm) \
	X(R10G10B10A2_UInt,    R10G10B10A2_UInt) \
	X(R11G11B10_Float,     R11G11B10_Float) \
	X(R8G8B8A8_UNorm,      R8G8B8A8_UNorm) \
	X(R8G8B8A8_UNorm_SRGB, R8G8B8A8_UNorm_SRGB) \
	X(R8G8B8A8_UInt,       R8G8B8A8_UInt) \
	X(R8G8B8A8_SNorm,      R8G8B8A8_SNorm) \
	X(R8G8B8A8_SInt,       R8G8B8A8_SInt) \
	X(R16G16_Float,        R16G16_Float) \
	X(R16G16_UNorm,        R16G16_UNorm) \
	X(R16G16_UInt,         R16G16_UInt) \
	X(R16G16_SNorm,        R16G16_SNorm) \
	X(R16G16_SInt,         R16G16_SInt) \
	X(R32_Depth,           D32_Float) \
	X(R32_Float,           R32_Float) \
	X(R32_UInt,            R32_UInt) \
	X(R32_SInt,            R32_SInt) \
	X(R8G8_UNorm,          R8G8_UNorm) \
	X(R8G8_UInt,           R8G8_UInt) \
	X(R8G8_SNorm,          R8G8_SNorm) \
	X(R8G8_SInt,           R8G8_SInt) \
	X(R16_Float,           R16_Float) \
	X(R16_Depth,           D16_UNorm) \
	X(R16_UNorm,           R16_UNorm) \
	X(R16_UInt,            R16_UInt) \
	X(R16_SNorm,           R16_SNorm) \
	X(R16_SInt,            R16_SInt) \
	X(R8_UNorm,            R8_UNorm) \
	X(R8_UInt,             R8_UInt) \
	X(R8_SNorm,            R8_SNorm) \
	X(R8_SInt,             R8_SInt) \
	X(R9G9B9E5_SharedExp,  R9G9B9E5_SharedExp) \
	X(BC1_UNorm,           BC1_UNorm) \
	X(BC1_UNorm_SRGB,      BC1_UNorm_SRGB) \
	X(BC2_UNorm,           BC2_UNorm) \
	X(BC2_UNorm_SRGB,      BC2_UNorm_SRGB) \
	X(BC3_UNorm,           BC3_UNorm) \
	X(BC3_UNorm_SRGB,      BC3_UNorm_SRGB) \
	X(BC4_UNorm,           BC4_UNorm) \
	X(BC4_SNorm,           BC4_SNorm) \
	X(BC5_UNorm,           BC5_UNorm) \
	X(BC5_SNorm,           BC5_SNorm) \
	X(B5G6R5_UNorm,        B5G6R5_UNorm) \
	X(B5G5R5A1_UNorm,      B5G5R5A1_UNorm) \
	X(B8G8R8A8_UNorm,      B8G8R8A8_UNorm) \
	X(B8G8R8A8_UNorm_SRGB, B8G8R8A8_UNorm_SRGB) \
	X(BC6H_UF16,           BC6H_UF16) \
	X(BC6H_SF16,           BC6H_SF16) \
	X(BC7_UNorm,           BC7_UNorm) \
	X(BC7_UNorm_SRGB,      BC7_UNorm_SRGB) \
	X(B4G4R4A4_UNorm,      B4G4R4A4_UNorm)

	constexpr FormatDDS GetDDSFormat(PixelFormat format) noexcept
	{
#define X(pixelFormat, ddsFormat) case PixelFormat::##pixelFormat: return FormatDDS::##ddsFormat;
		switch (format)
		{
		default:
			ZE_ENUM_UNHANDLED();
			ZE_DDS_FORMAT_MAPPINGS
		}
#undef X
	}

	constexpr PixelFormat GetFormatFromDDS(FormatDDS ddsFormat) noexcept
	{
#define X(pixelFormat, ddsFormat) case FormatDDS::##ddsFormat: return PixelFormat::##pixelFormat;
		switch (ddsFormat)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FormatDDS::R1_UNorm:
		case FormatDDS::R8G8_B8G8_UNorm:
		case FormatDDS::G8R8_G8B8_UNorm:
		case FormatDDS::B8G8R8A8_Typeless:
		case FormatDDS::B8G8R8X8_Typeless:
		case FormatDDS::B8G8R8X8_UNorm:
		case FormatDDS::R10G10B10_XR_Bias_A2_UNorm:
		case FormatDDS::B8G8R8X8_UNorm_SRGB:
			ZE_FAIL("Trying to convert unsupported format!");
			ZE_DDS_FORMAT_MAPPINGS
				// Convert typeless formats to proper types
		case FormatDDS::R32G32B32A32_Typeless: return PixelFormat::R32G32B32A32_UInt;
		case FormatDDS::R32G32B32_Typeless:    return PixelFormat::R32G32B32_UInt;
		case FormatDDS::R16G16B16A16_Typeless: return PixelFormat::R16G16B16A16_UInt;
		case FormatDDS::R32G32_Typeless:       return PixelFormat::R32G32_UInt;
		case FormatDDS::R10G10B10A2_Typeless:  return PixelFormat::R10G10B10A2_UInt;
		case FormatDDS::R8G8B8A8_Typeless:     return PixelFormat::R8G8B8A8_UInt;
		case FormatDDS::R16G16_Typeless:       return PixelFormat::R16G16_UInt;
		case FormatDDS::R32_Typeless:          return PixelFormat::R32_UInt;
		case FormatDDS::R8G8_Typeless:         return PixelFormat::R8G8_UInt;
		case FormatDDS::R16_Typeless:          return PixelFormat::R16_UInt;
		case FormatDDS::A8_UNorm:
		case FormatDDS::R8_Typeless:           return PixelFormat::R8_UInt;
		case FormatDDS::BC1_Typeless:          return PixelFormat::BC1_UNorm;
		case FormatDDS::BC2_Typeless:          return PixelFormat::BC2_UNorm;
		case FormatDDS::BC3_Typeless:          return PixelFormat::BC3_UNorm;
		case FormatDDS::BC4_Typeless:          return PixelFormat::BC4_UNorm;
		case FormatDDS::BC5_Typeless:          return PixelFormat::BC5_UNorm;
		case FormatDDS::BC6H_Typeless:         return PixelFormat::BC6H_UF16;
		case FormatDDS::BC7_Typeless:          return PixelFormat::BC7_UNorm;
		}
#undef X
	}
#pragma endregion
}