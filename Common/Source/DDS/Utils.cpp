#include "DDS/Utils.h"

namespace ZE::DDS
{
	static constexpr FormatDDS ParseDDSFormat(const PixelFormatDDS& format) noexcept
	{
#define ZE_IS_MASK(r, g, b, a) (format.RBitMask == r && format.GBitMask == g && format.BBitMask == b && format.ABitMask == a)
#define ZE_IS_FOURCC(c0, c1, c2, c3) (static_cast<U32>(c0) | (static_cast<U32>(c1) << 8) | (static_cast<U32>(c2) << 16) | (static_cast<U32>(c3) << 24)) == format.FourCC

		// SRGB formats are written using the DX10 extended header
		if (format.Flags & PixelFlag::RGB)
		{
			switch (format.RGBBitCount)
			{
			case 32:
			{
				if (ZE_IS_MASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)) return FormatDDS::R8G8B8A8_UNorm;
				if (ZE_IS_MASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000)) return FormatDDS::B8G8R8A8_UNorm;
				if (ZE_IS_MASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000)) return FormatDDS::B8G8R8X8_UNorm;
				// Only 32-bit color channel format in D3D9 was R32F, D3DX writes this out as a FourCC of 114
				if (ZE_IS_MASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000)) return FormatDDS::R32_Float;
				if (ZE_IS_MASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000)) return FormatDDS::R16G16_UNorm;
				if (ZE_IS_MASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000)) return FormatDDS::R10G10B10A2_UNorm;
				// Many common DDS reader/writers (including D3DX) swap the the RED/BLUE masks for 10:10:10:2 formats.
				// Original format is D3DFMT_A2R10G10B10 but no support for such format
				if (ZE_IS_MASK(0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000))
				{
					ZE_WARNING("Converting B10G10R10A2_UNorm DDS format to R10G10B10A2_UNorm!");
					return FormatDDS::R10G10B10A2_UNorm;
				}
				// No support for ZE_IS_MASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000), aka D3DFMT_X8B8G8R8
				break;
			}
			// No support 24 bits per pixel formats, aka D3DFMT_R8G8B8
			case 24:
				break;
			case 16:
			{
				if (ZE_IS_MASK(0x7c00, 0x03e0, 0x001f, 0x8000)) return FormatDDS::B5G5R5A1_UNorm;
				if (ZE_IS_MASK(0xf800, 0x07e0, 0x001f, 0x0000)) return FormatDDS::B5G6R5_UNorm;
				if (ZE_IS_MASK(0x0f00, 0x00f0, 0x000f, 0xf000)) return FormatDDS::B4G4R4A4_UNorm;

				// No support for ZE_IS_MASK(0x7c00, 0x03e0, 0x001f, 0x0000), aka D3DFMT_X1R5G5B5
				// No support for ZE_IS_MASK(0x0f00, 0x00f0, 0x000f, 0x0000), aka D3DFMT_X4R4G4B4
				// No 3:3:2, 3:3:2:8, or paletted formats, aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
				break;
			}
			}
		}
		else if (format.Flags & PixelFlag::Luminance)
		{
			if (8 == format.RGBBitCount)
			{
				// // D3DX10/11 writes this out as DX10 extension
				if (ZE_IS_MASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000)) return FormatDDS::R8_UNorm;

				// No support for ZE_IS_MASK(0x0f, 0x00, 0x00, 0xf0), aka D3DFMT_A4L4
			}
			else if (16 == format.RGBBitCount)
			{
				// D3DX10/11 writes this out as DX10 extension
				if (ZE_IS_MASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000)) return FormatDDS::R16_UNorm;
				// D3DX10/11 writes this out as DX10 extension
				if (ZE_IS_MASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00)) return FormatDDS::R8G8_UNorm;
			}
		}
		else if (format.Flags & PixelFlag::Alpha)
		{
			if (8 == format.RGBBitCount)
				return FormatDDS::A8_UNorm;
		}
		else if (format.Flags & PixelFlag::FourCC)
		{
			if (ZE_IS_FOURCC('D', 'X', 'T', '1')) return FormatDDS::BC1_UNorm;
			if (ZE_IS_FOURCC('D', 'X', 'T', '3')) return FormatDDS::BC2_UNorm;
			if (ZE_IS_FOURCC('D', 'X', 'T', '5')) return FormatDDS::BC3_UNorm;

			// While pre-mulitplied alpha isn't directly supported,
			// they are basically the same as these BC formats so they can be mapped
			if (ZE_IS_FOURCC('D', 'X', 'T', '2')) return FormatDDS::BC2_UNorm;
			if (ZE_IS_FOURCC('D', 'X', 'T', '4')) return FormatDDS::BC3_UNorm;

			if (ZE_IS_FOURCC('A', 'T', 'I', '1')) return FormatDDS::BC4_UNorm;
			if (ZE_IS_FOURCC('B', 'C', '4', 'U')) return FormatDDS::BC4_UNorm;
			if (ZE_IS_FOURCC('B', 'C', '4', 'S')) return FormatDDS::BC4_SNorm;

			if (ZE_IS_FOURCC('A', 'T', 'I', '2')) return FormatDDS::BC5_UNorm;
			if (ZE_IS_FOURCC('B', 'C', '5', 'U')) return FormatDDS::BC5_UNorm;
			if (ZE_IS_FOURCC('B', 'C', '5', 'S')) return FormatDDS::BC5_SNorm;

			// BC6H and BC7 are written using the DX10 extended header

			if (ZE_IS_FOURCC('R', 'G', 'B', 'G')) return FormatDDS::R8G8_B8G8_UNorm;
			if (ZE_IS_FOURCC('G', 'R', 'G', 'B')) return FormatDDS::G8R8_G8B8_UNorm;

			// Check for D3DFORMAT enums being set here.
			switch (format.FourCC)
			{
				// D3DFMT_A16B16G16R16
			case 36: return FormatDDS::R16G16B16A16_UNorm;
				// D3DFMT_Q16W16V16U16
			case 110: return FormatDDS::R16G16B16A16_SNorm;
				// D3DFMT_R16F
			case 111: return FormatDDS::R16_Float;
				// D3DFMT_G16R16F
			case 112: return FormatDDS::R16G16_Float;
				// D3DFMT_A16B16G16R16F
			case 113: return FormatDDS::R16G16B16A16_Float;
				// D3DFMT_R32F
			case 114: return FormatDDS::R32_Float;
				// D3DFMT_G32R32F
			case 115: return FormatDDS::R32G32_Float;
				// D3DFMT_A32B32G32R32F
			case 116: return FormatDDS::R32G32B32A32_Float;
			}
		}

		return FormatDDS::Unknown;
#undef ZE_IS_FOURCC
#undef ZE_IS_MASK
	}

	static constexpr void GetSurfaceInfo(U32 width, U32 height, PixelFormat format, U32& rowSize, U32& rowCount) noexcept
	{
		bool blockCompression = false;
		U8 bytePairEncoding = 0;
		switch (format)
		{
		case PixelFormat::BC1_UNorm:
		case PixelFormat::BC1_UNorm_SRGB:
		case PixelFormat::BC4_UNorm:
		case PixelFormat::BC4_SNorm:
		{
			blockCompression = true;
			bytePairEncoding = 8;
			break;
		}
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
		{
			blockCompression = true;
			bytePairEncoding = 16;
			break;
		}
		default:
			break;
		}

		if (blockCompression)
		{
			rowSize = std::max(1U, (width + 3) / 4) * bytePairEncoding;
			rowCount = std::max(1U, (height + 3) / 4);
		}
		else
		{
			rowSize = (width * Utils::GetFormatBitCount(format) + 7) / 8;
			rowCount = height;
		}
	}

	FileResult ParseFile(FILE* file, FileData& destData, U32 destRowAlignment, U32 destSliceAlignment) noexcept
	{
#define ZE_DDS_CHECK_READ(item) if (fread(&item, sizeof(item), 1, file) != 1) return FileResult::ReadError
#define ZE_IS_FOURCC(c0, c1, c2, c3) (static_cast<U32>(c0) | (static_cast<U32>(c1) << 8) | (static_cast<U32>(c2) << 16) | (static_cast<U32>(c3) << 24)) == header.Format.FourCC

		// DDS format definition: https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-pguide

		// Check for magic number
		U32 magic = 0;
		ZE_DDS_CHECK_READ(magic);
		if (magic != MAGIC_NUMBER)
			return FileResult::IncorrectMagicNumber;

		Header header = {};
		ZE_DDS_CHECK_READ(header);

		// Read DXT10 header and perform integrity checks
		PixelFormat format = PixelFormat::Unknown;
		U16 arraySize = 1;
		U16 depth = 1;
		if (ZE_IS_FOURCC('D', 'X', '1', '0'))
		{
			HeaderDXT10 dxt10Header = {};
			ZE_DDS_CHECK_READ(dxt10Header);

			if (dxt10Header.ArraySize == 0)
				return FileResult::IncorrectArraySize;
			arraySize = Utils::SafeCast<U16>(dxt10Header.ArraySize);
			format = GetFormatFromDDS(dxt10Header.Format);

			if (dxt10Header.Dimension & ResourceDimension::Texture3D)
			{
				if (!(header.Caps2 & HeaderCap2::Volume) || dxt10Header.ArraySize > 1)
					return FileResult::IllformattedVolumeTexture;
				depth = Utils::SafeCast<U16>(header.Depth);
			}
			else if (dxt10Header.Dimension & ResourceDimension::Texture2D)
			{
				if ((dxt10Header.Dimension & ResourceDimension::Texture2D) && (dxt10Header.MiscFlag & MiscFlagDXT10::TextureCube))
					arraySize *= 6;
			}
			else if (dxt10Header.Dimension & ResourceDimension::Texture1D)
			{
				if ((header.Flags & HeaderFlag::Height) && header.Height != 1)
					return FileResult::Incorrect1DTextureHeight;
			}
			else
				return FileResult::IncorrectDimension;
		}
		else
		{
			format = GetFormatFromDDS(ParseDDSFormat(header.Format));
			if (header.Caps2 & HeaderCap2::Volume)
				depth = Utils::SafeCast<U16>(header.Depth);
			else if (header.Caps2 & HeaderCap2::Cubemap)
			{
				// For older header types there can be situation where not all faces are defined, need to check for that (D3D9 era)
				if ((header.Caps2 & HeaderCap2::CubemapAllFaces) != HeaderCap2::CubemapAllFaces)
					return FileResult::MissingCubemapFaces;
				arraySize = 6;
			}
		}
		if (format == PixelFormat::Unknown)
			return FileResult::UnknownFormat;

		// Compute padded destination image size
		U64 destImageSize = 0;
		const U16 mipCount = Utils::SafeCast<U16>(header.MipMapCount ? header.MipMapCount : 1);
		for (U16 a = 0; a < arraySize; ++a)
		{
			U32 currentWidth = header.Width;
			U32 currentHeight = header.Height;
			U16 currentDepth = depth;
			for (U16 mip = 0; mip < mipCount; ++mip)
			{
				U32 rowSize, rowCount;
				GetSurfaceInfo(currentWidth, currentHeight, format, rowSize, rowCount);

				destImageSize += Math::AlignUp(Math::AlignUp(rowSize, destRowAlignment) * rowCount, destSliceAlignment) * currentDepth;

				currentWidth >>= 1;
				if (currentWidth == 0)
					currentWidth = 1;
				currentHeight >>= 1;
				if (currentHeight == 0)
					currentHeight = 1;
				currentDepth >>= 1;
				if (currentDepth == 0)
					currentDepth = 1;
			}
		}

		// Read surfaces from disk to memory directly in padded regions
		std::shared_ptr<U8[]> image = std::make_shared<U8[]>(destImageSize);
		U8* destImageMemory = image.get();
		for (U16 a = 0; a < arraySize; ++a)
		{
			U32 currentWidth = header.Width;
			U32 currentHeight = header.Height;
			U16 currentDepth = depth;
			for (U16 mip = 0; mip < mipCount; ++mip)
			{
				U32 rowSize, rowCount;
				GetSurfaceInfo(currentWidth, currentHeight, format, rowSize, rowCount);

				// Check if single images or whole depth level can be read at once
				const bool sameRowAlignment = (rowSize % destRowAlignment) == 0;
				const U32 sliceSize = rowSize * rowCount;
				if (sameRowAlignment && (sliceSize % destSliceAlignment) == 0)
				{
					const U32 depthLevelSize = currentDepth * sliceSize;
					if (fread(destImageMemory, depthLevelSize, 1, file) != 1)
						return FileResult::ReadError;
					destImageMemory += depthLevelSize;
				}
				else
				{
					const U32 destRowSize = Math::AlignUp(rowSize, destRowAlignment);
					for (U32 depthSlice = 0; depthSlice < currentDepth; ++depthSlice)
					{
						if (sameRowAlignment)
						{
							if (fread(destImageMemory, sliceSize, 1, file) != 1)
								return FileResult::ReadError;
							destImageMemory += sliceSize;
						}
						else
						{
							for (U32 row = 0; row < rowCount; ++row)
							{
								if (fread(destImageMemory, rowSize, 1, file) != 1)
									return FileResult::ReadError;
								destImageMemory += destRowSize;
							}
						}
					}
				}

				currentWidth >>= 1;
				if (currentWidth == 0)
					currentWidth = 1;
				currentHeight >>= 1;
				if (currentHeight == 0)
					currentHeight = 1;
				currentDepth >>= 1;
				if (currentDepth == 0)
					currentDepth = 1;
			}
		}

		destData.Format = format;
		destData.Width = header.Width;
		destData.Height = header.Height;
		destData.Depth = depth;
		destData.MipCount = mipCount;
		destData.ArraySize = arraySize;
		destData.ImageMemorySize = Utils::SafeCast<U32>(destImageSize);
		destData.ImageMemory = image;
		return FileResult::Ok;
#undef ZE_IS_FOURCC
#undef ZE_DDS_CHECK_READ
	}
}