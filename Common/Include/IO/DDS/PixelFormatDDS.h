#pragma once
#include "Macros.h"

namespace ZE::IO::DDS
{
	// Flags used in DDS::PixelFormatDDS
	typedef U32 PixelFlags;
	// Possible flags describing contents of DDS::PixelFormatDDS
	enum class PixelFlag : PixelFlags
	{
		// Texture contains alpha data, R/G/B/ABitMask contains valid data
		AlphaPixels = 1,
		// Used in some older DDS files for alpha channel only uncompressed data (RGBBitCount contains the alpha channel bitcount, ABitMask contains valid data)
		Alpha = 2,
		// Texture contains compressed RGB data, FourCC contains valid data
		FourCC = 4,
		// Texture contains uncompressed RGB data, RGBBitCount and the R/G/BBitMask contain valid data
		RGB = 0x40,
		// Used in some older DDS files for YUV uncompressed data (RGBBitCount contains the YUV bit count, RBitMask contains the Y mask, GBitMask contains the U mask, BBitMask contains the V mask)
		YUV = 0x200,
		// Used in some older DDS files for single channel color uncompressed data (RGBBitCount contains the luminance channel bit count, RBitMask contains the channel mask).
		// Can be combined with AlphaPixels for a two channel DDS file
		Luminance = 0x20000
	};
	ZE_ENUM_OPERATORS(PixelFlag, PixelFlags);

#pragma pack(push, 1)
	// Description of pixel format in DDS texture
	struct PixelFormatDDS
	{
		// Structure size, must be 32
		U32 Size;
		PixelFlags Flags;
		// Four-character codes for specifying compressed or custom formats.
		// Possible values include: DXT1, DXT2, DXT3, DXT4, or DXT5.
		// A FourCC of DX10 indicates the prescense of the HeaderDXT10 extended header,
		// and the Format member of that structure indicates the true format.
		// Valid with PixelFlag::FourCC
		U32 FourCC;
		// Valid with PixelFlag::RGB, PixelFlag::YUV or PixelFlag::Luminance
		U32 RGBBitCount;
		U32 RBitMask;
		U32 GBitMask;
		U32 BBitMask;
		// Valid with PixelFlag::AlphaPixels or PixelFlag::Alpha
		U32 ABitMask;
	};
#pragma pack(pop)

	static_assert(sizeof(PixelFormatDDS) == 32, "Incorret size of DDS pixel format header!");
}