#include "GFX/Surface.h"
#include "DDS/Utils.h"
ZE_WARNING_PUSH
#include "spng.h"
#include "qoixx.hpp"
#define STBI_ASSERT(X) ZE_ASSERT(X, "STB Image assert!")
#define STB_IMAGE_STATIC
#define STBI_FAILURE_USERMSG
#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBIW_ASSERT(X) ZE_ASSERT(X, "STB Image Write assert!")
#define STBIW_WINDOWS_UTF8
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	template<Surface::CopySource SRC_FORMAT, typename T>
	void Surface::CopyLoadedImage(T* destImage, const T* srcImage, U32 width, U32 height, U32 destRowSize) noexcept
	{
		constexpr U32 SRC_PIXEL_SIZE = 2 + (SRC_FORMAT == CopySource::RGB) * sizeof(T);
		for (U32 y = 0; y < height; ++y)
		{
			for (U32 x = 0; x < width; ++x)
			{
				const U32 destOffset = x * 4 * sizeof(T);
				const U32 srcOffset = x * SRC_PIXEL_SIZE;
				destImage[destOffset] = srcImage[srcOffset];

				if constexpr (SRC_FORMAT == CopySource::GrayscaleAlpha)
				{
					// Expand grayscale alpha image to RGBA
					destImage[destOffset + 1] = srcImage[srcOffset];
					destImage[destOffset + 2] = srcImage[srcOffset];
					destImage[destOffset + 3] = srcImage[srcOffset + 1];
				}
				else if constexpr (SRC_FORMAT == CopySource::RGB)
				{
					// Expand RGB image to RGBA
					destImage[destOffset + 1] = srcImage[srcOffset + 1];
					destImage[destOffset + 2] = srcImage[srcOffset + 2];
					// Set alpha as 1 when reading from texture
					if constexpr (std::is_same_v<T, float>)
						destImage[destOffset + 3] = 1.0f;
					else
						destImage[destOffset + 3] = static_cast<T>(UINT64_MAX);
				}
				else
				{
					ZE_FAIL("Unhandled Surface copy source type!");
				}
			}
			// Move to next row with correct padding
			destImage = reinterpret_cast<T*>(reinterpret_cast<U8*>(destImage) + destRowSize);
			srcImage = reinterpret_cast<const T*>(reinterpret_cast<const U8*>(srcImage) + width * SRC_PIXEL_SIZE);
		}
	}

	Surface::Surface(U32 width, U32 height, PixelFormat format, const void* srcImage) noexcept
		: format(format), width(width), height(height), depth(1), mipCount(1), arraySize(1),
		memorySize(GetSliceByteSize()), memory(std::make_shared<U8[]>(memorySize))
	{
		if (srcImage)
		{
			const U32 destRowSize = GetRowByteSize();
			const U32 srcRowSize = width * GetPixelSize();
			if (destRowSize == srcRowSize)
				std::memcpy(memory.get(), srcImage, srcRowSize);
			else
			{
				for (U32 row = 0; row < height; ++row)
					std::memcpy(memory.get() + row * destRowSize, reinterpret_cast<const U8*>(srcImage) + row * srcRowSize, srcRowSize);
			}
		}
	}

	bool Surface::Load(std::string_view filename) noexcept
	{
		const std::filesystem::path path(filename);
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return static_cast<char>(std::tolower(c)); });

		FILE* file = fopen(filename.data(), "rb");
		if (!file)
		{
			Logger::Error("Error openinig \"" + path.string() + "\" file!");
			return false;
		}

		depth = 1;
		mipCount = 1;
		arraySize = 1;
		bool success = false;
		bool tryStbi = true;
		bool checkForAlpha = false;
		if (ext == ".dds")
		{
			tryStbi = false;
			DDS::FileData ddsData = {};
			switch (DDS::ParseFile(file, ddsData, ROW_PITCH_ALIGNMENT, SLICE_PITCH_ALIGNMENT))
			{
			case DDS::FileResult::Ok:
			{
				success = true;
				format = ddsData.Format;
				// Don't investigate further, trust the loader till support for BC textures implemented
				alpha = ddsData.Alpha;
				width = ddsData.Width;
				height = ddsData.Height;
				depth = ddsData.Depth;
				mipCount = ddsData.MipCount;
				arraySize = ddsData.ArraySize;
				memorySize = ddsData.ImageMemorySize;
				memory = ddsData.ImageMemory;
				break;
			}
			default:
				ZE_ENUM_UNHANDLED();
			case DDS::FileResult::ReadError:
				Logger::Error("Error reading DDS file \"" + path.string() + "\"!");
				break;
			case DDS::FileResult::IncorrectMagicNumber:
				Logger::Error("Error reading DDS file \"" + path.string() + "\", incorrect DDS magic number!");
				break;
			case DDS::FileResult::UnknownFormat:
				Logger::Error("Error reading DDS file \"" + path.string() + "\", not supported pixel format!");
				break;
			case DDS::FileResult::MissingCubemapFaces:
				Logger::Error("Error reading DDS file \"" + path.string() + "\", not all cubemap faces defined!");
				break;
			case DDS::FileResult::IllformattedVolumeTexture:
				Logger::Error("Error reading DDS file \"" + path.string() + "\", incorrectly formatted volume texture (ex. array size bigger than 1)!");
				break;
			case DDS::FileResult::IncorrectArraySize:
				Logger::Error("Error reading DDS file \"" + path.string() + "\", wrong value used as array size!");
				break;
			case DDS::FileResult::Incorrect1DTextureHeight:
				Logger::Error("Error reading DDS file \"" + path.string() + "\", 1D texture with height different than 1!");
				break;
			case DDS::FileResult::IncorrectDimension:
				Logger::Error("Error reading DDS file \"" + path.string() + "\", unknown texture dimension!");
				break;
			}
		}
		else if (ext == ".png")
		{
			spng_ctx* ctx = spng_ctx_new(0);
			ZE_ASSERT(ctx, "Error creating SPNG context!");

			spng_ihdr header = {};
			int result = spng_set_png_file(ctx, file);
			if (!result)
			{
				result = spng_get_ihdr(ctx, &header);
				if (!result)
				{
					int conversionFormat;
					if (header.color_type == SPNG_COLOR_TYPE_GRAYSCALE)
					{
						format = PixelFormat::R8_UNorm;
						conversionFormat = SPNG_FMT_G8;
					}
					else if (header.bit_depth == 16)
					{
						format = PixelFormat::R16G16B16A16_UNorm;
						conversionFormat = SPNG_FMT_RGBA16;
					}
					else
					{
						format = PixelFormat::R8G8B8A8_UNorm;
						conversionFormat = SPNG_FMT_RGBA8;
					}

					width = header.width;
					height = header.height;
					alpha = header.color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA || header.color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA;
					checkForAlpha = alpha;

					const U32 destRowSize = GetRowByteSize();
					const U32 srcRowSize = width * GetPixelSize();
					memorySize = GetSliceByteSize();
					memory = std::make_shared<U8[]>(memorySize);

					// Write directly to the buffer as padding is not required
					if (destRowSize == srcRowSize)
						result = spng_decode_image(ctx, memory.get(), memorySize, conversionFormat, SPNG_DECODE_TRNS);
					else
					{
						// Read image progressively row by row into final buffer
						result = spng_decode_image(ctx, nullptr, 0, conversionFormat, SPNG_DECODE_TRNS | SPNG_DECODE_PROGRESSIVE);
						if (!result)
						{
							spng_row_info rowInfo;
							do
							{
								result = spng_get_row_info(ctx, &rowInfo);
								if (!result)
									result = spng_decode_row(ctx, memory.get() + width * rowInfo.row_num, srcRowSize);
							} while (!result);
							if (result == SPNG_EOI)
								result = SPNG_OK;
						}
					}
				}
			}
			spng_ctx_free(ctx);
			if (result)
			{
				Logger::Warning("Error loading file \"" + path.string() + "\", trying fallback to STB Image, SPNG error: " + std::string(spng_strerror(result)));
				memory = nullptr;
				rewind(file);
			}
			else
				success = true;
		}
		else if (ext == ".qoi")
		{
			tryStbi = false;
			const U64 fileSize = std::filesystem::file_size(path);
			if (fileSize)
			{
				std::vector<U8> srcImage(fileSize);
				if (fread(srcImage.data(), 1, fileSize, file) == 1)
				{
					auto result = qoixx::qoi::decode<std::vector<U8>>(srcImage);

					alpha = result.second.channels == 4;
					width = result.second.width;
					height = result.second.height;
					if (result.second.colorspace == qoixx::qoi::colorspace::srgb)
						format = PixelFormat::R8G8B8A8_UNorm_SRGB;
					else
						format = PixelFormat::R8G8B8A8_UNorm;
					memorySize = GetSliceByteSize();
					memory = std::make_shared<U8[]>(memorySize);

					const U32 destRowSize = GetRowByteSize();
					if (result.second.channels == 3)
						CopyLoadedImage<CopySource::RGB>(memory.get(), result.first.data(), width, height, destRowSize);
					else
					{
						ZE_ASSERT(result.second.channels == 4, "According to QOI spec, only allowed formats are RGB and RGBA!");
						checkForAlpha = true;

						// When rows don't require padding then copy it directly, otherwise row by row
						const U32 srcRowSize = width * result.second.channels;
						if (srcRowSize == destRowSize)
							std::memcpy(memory.get(), result.first.data(), srcRowSize);
						else
						{
							for (U32 y = 0; y < height; ++y)
								std::memcpy(memory.get() + y * destRowSize, result.first.data() + y * srcRowSize, srcRowSize);
						}
					}
				}
				else
				{
					if (feof(file))
						Logger::Error("Error reading file \"" + path.string() + "\", unexpected end of file!");
					else
						Logger::Error("Error reading file \"" + path.string() + "\"!");
					success = false;
				}
			}
			else
			{
				Logger::Error("Error reading file \"" + path.string() + "\", file with 0 size!");
				success = false;
			}
		}

		// Last resort method to try loading image file
		if (!success && tryStbi)
		{
			if (ext != ".jpeg" && ext != ".jpg" && ext != ".bmp" && ext != ".psd" && ext != ".tga" && ext != ".png"
				&& ext != ".gif" && ext != ".hdr" && ext != ".pic" && ext != ".pgm" && ext != ".ppm")
				Logger::Warning("Unknown format for file \"" + path.string() + "\"! Trying to load via STB Image anyway.");

			int srcWidth = 0, srcHeight = 0, components = 0;
			U8* srcImage = nullptr;
			PixelFormat imageFormat = PixelFormat::Unknown;
			bool expandAlpha = false;

			if (stbi_is_hdr_from_file(file))
			{
				// More handling might needed when RGBE is used
				srcImage = reinterpret_cast<U8*>(stbi_loadf_from_file(file, &srcWidth, &srcHeight, &components, 0));
				switch (components)
				{
				default:
					ZE_ENUM_UNHANDLED();
				case 1:
					imageFormat = PixelFormat::R32_Float;
					break;
				case 3:
					imageFormat = PixelFormat::R32G32B32_Float;
					break;
				case 2:
					expandAlpha = true;
					[[fallthrough]];
				case 4:
					imageFormat = PixelFormat::R32G32B32A32_Float;
					break;
				}
			}
			if (!srcImage && stbi_is_16_bit_from_file(file))
			{
				srcImage = reinterpret_cast<U8*>(stbi_load_from_file_16(file, &srcWidth, &srcHeight, &components, 0));
				switch (components)
				{
				default:
					ZE_ENUM_UNHANDLED();
				case 1:
					imageFormat = PixelFormat::R16_UNorm;
					break;
				case 2:
				case 3:
					expandAlpha = true;
					[[fallthrough]];
				case 4:
					imageFormat = PixelFormat::R16G16B16A16_UNorm;
					break;
				}
			}
			else if (!srcImage)
			{
				srcImage = stbi_load_from_file(file, &srcWidth, &srcHeight, &components, 0);
				switch (components)
				{
				default:
					ZE_ENUM_UNHANDLED();
				case 1:
					imageFormat = PixelFormat::R8_UNorm;
					break;
				case 2:
				case 3:
					expandAlpha = true;
					[[fallthrough]];
				case 4:
					imageFormat = PixelFormat::R8G8B8A8_UNorm;
					break;
				}
			}

			if (srcImage)
			{
				ZE_ASSERT(imageFormat != PixelFormat::Unknown, "Incorrect pixel format!");
				format = imageFormat;
				width = static_cast<U32>(srcWidth);
				height = static_cast<U32>(srcHeight);
				memorySize = GetSliceByteSize();
				memory = std::make_shared<U8[]>(memorySize);
				alpha = components == 4;
				checkForAlpha = alpha;

				const U32 destRowSize = GetRowByteSize();
				if (expandAlpha)
				{
					// Copy image and transform it into 4-component version
					const U8 channelSize = GetPixelSize() / 4;
					if (components == 2)
					{
						// Copy grayscale image with alpha
						switch (channelSize)
						{
						default:
							ZE_ENUM_UNHANDLED();
						case 1:
							CopyLoadedImage<CopySource::GrayscaleAlpha>(memory.get(), srcImage, width, height, destRowSize);
							break;
						case 2:
							CopyLoadedImage<CopySource::GrayscaleAlpha>(reinterpret_cast<U16*>(memory.get()), reinterpret_cast<const U16*>(srcImage), width, height, destRowSize);
							break;
						case 4:
							CopyLoadedImage<CopySource::GrayscaleAlpha>(reinterpret_cast<float*>(memory.get()), reinterpret_cast<const float*>(srcImage), width, height, destRowSize);
							break;
						}
					}
					else
					{
						// Copy RGB image with alpha as 1
						ZE_ASSERT(components == 3, "Incorrect number of components!");
						switch (channelSize)
						{
						default:
							ZE_ENUM_UNHANDLED();
						case 1:
							CopyLoadedImage<CopySource::RGB>(memory.get(), srcImage, width, height, destRowSize);
							break;
						case 2:
							CopyLoadedImage<CopySource::RGB>(reinterpret_cast<U16*>(memory.get()), reinterpret_cast<const U16*>(srcImage), width, height, destRowSize);
							break;
						case 4:
							CopyLoadedImage<CopySource::RGB>(reinterpret_cast<float*>(memory.get()), reinterpret_cast<const float*>(srcImage), width, height, destRowSize);
							break;
						}
					}
				}
				else
				{
					// When rows don't require padding then copy it directly, otherwise row by row
					const U32 srcRowSize = width * GetPixelSize();
					if (srcRowSize == destRowSize)
						std::memcpy(memory.get(), srcImage, srcRowSize * height);
					else
					{
						for (U32 y = 0; y < height; ++y)
							std::memcpy(memory.get() + y * destRowSize, srcImage + y * srcRowSize, srcRowSize);
					}
				}
				stbi_image_free(srcImage);
				success = true;
			}
			else
				Logger::Error("Error loading file \"" + path.string() + "\", STB Image error: " + std::string(stbi_failure_reason()));
		}
		fclose(file);

		// When original file contains alpha then check if it's not all opaque
		// to avoid setting this texture as source of transparency (only for basic types)
		if (checkForAlpha)
		{
			for (U32 y = 0; y < height; ++y)
			{
				const U32 rowOffset = y * GetRowByteSize();
				for (U32 x = 0; x < width; ++x)
				{
					const U32 offset = rowOffset + x * GetPixelSize() + (Utils::GetChannelCount(format) - 1) * Utils::GetChannelSize(format);
					U32 alphaChannel = 0;
					for (U8 p = 0; p < Utils::GetChannelSize(format); ++p)
						alphaChannel |= static_cast<U32>(memory[offset + p]) << p;

					if (Utils::GetAlpha(alphaChannel, format) != 1.0f)
						return success;
				}
			}
			alpha = false;
		}
		return success;
	}

	bool Surface::Save(std::string_view filename) const noexcept
	{
		const std::filesystem::path path = filename;
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return static_cast<char>(std::tolower(c)); });

		FILE* file = fopen(filename.data(), "rwb");
		if (!file)
		{
			Logger::Error("Error creating \"" + path.string() + "\" file!");
			return false;
		}

		bool success = true;
		if (ext == ".dds")
		{
		}
		else if (ext == ".png")
		{
			spng_ctx* ctx = spng_ctx_new(SPNG_CTX_ENCODER);
			ZE_ASSERT(ctx, "Error creating SPNG context!");

			int result = spng_set_png_file(ctx, file);
			if (!result)
			{
				// Need to check what formats can be saved directly and which one require per pixel copy
				spng_ihdr header = {};
				header.width = width;
				header.height = height;
				uint8_t bit_depth;
				uint8_t color_type; // SPNG_COLOR_TYPE_GRAYSCALE, SPNG_COLOR_TYPE_TRUECOLOR, SPNG_COLOR_TYPE_TRUECOLOR_ALPHA
				header.compression_method = 0;
				header.filter_method = 0;
				header.interlace_method = SPNG_INTERLACE_NONE;
				result = spng_set_ihdr(ctx, &header);
				if (!result)
				{
					const U32 srcRowSize = GetRowByteSize();
					const U32 destRowSize = width * GetPixelSize();
					if (destRowSize == srcRowSize)
						result = spng_encode_image(ctx, memory.get(), height * srcRowSize, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
					else
					{
						result = spng_encode_image(ctx, memory.get(), height * srcRowSize, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE | SPNG_ENCODE_PROGRESSIVE);
						if (!result)
						{
							for (U32 y = 0; y < height && !result; ++y)
								result = spng_encode_row(ctx, memory.get() + y * srcRowSize, destRowSize);
							if (result == SPNG_EOI)
								result = SPNG_OK;
						}
					}
				}
			}
			spng_ctx_free(ctx);
			if (result)
			{
				Logger::Error("Error writing file \"" + path.string() + "\", SPNG error: " + std::string(spng_strerror(result)));
				success = false;
			}
		}
		else if (ext == ".qoi")
		{
			qoixx::qoi::desc desc = {};
			std::vector<U8> encoded = qoixx::qoi::encode<std::vector<U8>>(memory.get(), 0, desc);
		}
		else if (ext == ".bmp")
		{
			//stbi_write_bmp();
		}
		else if (ext == ".tga")
		{
			//stbi_write_tga();
		}
		else if (ext == ".hdr")
		{
			//stbi_write_hdr();
		}
		else if (ext == ".jpg")
		{
			//stbi_write_jpg();
		}
		Logger::Error("Saving surface to \"" + path.string() + "\": failed to save.");

		fclose(file);
		return success;
	}

	U8* Surface::GetImage(U16 arrayIndex, U16 mipIndex, U16 depthLevel) noexcept
	{
		if (arrayIndex >= arraySize || mipIndex >= mipCount || depthLevel >= depth)
			return nullptr;
		if (mipCount == 1)
			return memory.get() + (arrayIndex + depthLevel) * GetSliceByteSize();

		// Complex path, move by specified mip levels and to selected depth
		auto getMipOffset = [&](U16 mipLevels, U16 depthLevel) -> U64
			{
				U64 offset = 0;
				U32 currentWidth = width;
				U32 currentHeight = height;
				U16 currentDepth = depth;
				for (U16 mip = 0; mip < mipLevels; )
				{
					offset += Math::AlignUp(Math::AlignUp((currentWidth * Utils::GetFormatBitCount(format)) / 8, ROW_PITCH_ALIGNMENT) * currentHeight, SLICE_PITCH_ALIGNMENT) * currentDepth;

					if (++mip < mipLevels)
					{
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
				return offset + depthLevel * Math::AlignUp(Math::AlignUp((currentWidth * Utils::GetFormatBitCount(format)) / 8, ROW_PITCH_ALIGNMENT) * currentHeight, SLICE_PITCH_ALIGNMENT);
			};
		U8* image = memory.get();
		for (U16 a = 0; a < arrayIndex; ++a)
			image += getMipOffset(mipCount, 0);
		return image + getMipOffset(mipIndex, depthLevel);
	}
}