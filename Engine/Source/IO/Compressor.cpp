#include "IO/Compressor.h"
ZE_WARNING_PUSH
#include "zlib.h"
#include "bzlib.h"
ZE_WARNING_POP

namespace ZE::IO
{
	U32 Compressor::GetOriginalSize(const void* compressedBuffer, U32 compressedSize) const noexcept
	{
		switch (format)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case CompressionFormat::None:
		{
			ZE_WARNING("Unoptimal code path, codec shouldn't be used without compression!");
			return compressedSize;
		}
		case CompressionFormat::ZLib:
		case CompressionFormat::Bzip2:
			return *reinterpret_cast<const U32*>(reinterpret_cast<const U8*>(compressedBuffer) + compressedSize - sizeof(U32) - 1);
		}
	}

	std::vector<U8> Compressor::Compress(const void* input, U32 inputSize) const noexcept
	{
		std::vector<U8> compressed;
		switch (format)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case CompressionFormat::None:
		{
			ZE_WARNING("Unoptimal code path, compression codec shouldn't be used without compression!");
			compressed.resize(inputSize);
			std::memcpy(compressed.data(), input, inputSize);
			break;
		}
		case CompressionFormat::ZLib:
		{
			z_stream strm = {};
			strm.next_in = reinterpret_cast<const U8*>(input);
			strm.avail_in = inputSize;
			strm.zalloc = nullptr;
			strm.zfree = nullptr;
			strm.opaque = nullptr;
			strm.data_type = Z_BINARY;

			// [9..15], [1..9], Z_RLE should be good for image data
			[[maybe_unused]] S32 ret = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, 15, 9, Z_DEFAULT_STRATEGY);
			ZE_ASSERT(ret == Z_OK, "Error initializing ZLIB deflate compression!");

			// Get max size after decompression
			strm.avail_out = deflateBound(&strm, inputSize);
			compressed.resize(strm.avail_out);
			strm.next_out = compressed.data();

			ret = deflate(&strm, Z_FINISH);
			ZE_ASSERT(ret == Z_STREAM_END && strm.avail_in == 0, "Error performing ZLIB deflate compression!");

			ret = deflateEnd(&strm);
			ZE_ASSERT(ret == Z_OK, "Error ending ZLIB deflate compression!");

			// Resize to actual size and append original file size (2 bytes added at the end of stream so there would be no data errors)
			compressed.resize(static_cast<U64>(strm.next_out - compressed.data()) + sizeof(U32) + 2);
			*reinterpret_cast<U32*>(&compressed.at(compressed.size() - sizeof(U32) - 1)) = inputSize;
			break;
		}
		case CompressionFormat::Bzip2:
		{
			// Can't determine lower bound as in Zlib so assume at least current size
			compressed.resize(inputSize);

			U32 compressedSize = inputSize;
			[[maybe_unused]] S32 ret = BZ2_bzBuffToBuffCompress(reinterpret_cast<char*>(compressed.data()), &compressedSize,
				reinterpret_cast<char*>(const_cast<void*>(input)), inputSize, 9, 0, 0);
			ZE_ASSERT(ret == BZ_OK, "Error performing Bzip2 compression!");

			// Resize to actual size and append original file size (2 bytes added at the end of stream so there would be no data errors)
			compressed.resize(compressedSize + sizeof(U32) + 2);
			*reinterpret_cast<U32*>(&compressed.at(compressed.size() - sizeof(U32) - 1)) = inputSize;
			break;
		}
		}
		return compressed;
	}

	void Compressor::Decompress(const void* src, U32 srcSize, void* dst, U32 dstSize) const noexcept
	{
		switch (format)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case CompressionFormat::None:
		{
			ZE_WARNING("Unoptimal code path, decompression codec shouldn't be used without compression!");
			ZE_ASSERT(srcSize == dstSize, "For not compressed data compressed and decompressed size should be equal!");
			std::memcpy(dst, src, dstSize);
			break;
		}
		case CompressionFormat::ZLib:
		{
			z_stream strm = {};
			strm.next_in = reinterpret_cast<const U8*>(src);
			strm.avail_in = srcSize - sizeof(U32);
			strm.zalloc = nullptr;
			strm.zfree = nullptr;
			strm.opaque = nullptr;
			strm.data_type = Z_BINARY;

			[[maybe_unused]] S32 ret = inflateInit2(&strm, 15);
			ZE_ASSERT(ret == Z_OK, "Error initializing ZLIB inflate decompression!");

			strm.next_out = reinterpret_cast<U8*>(dst);
			strm.avail_out = dstSize;
			ret = inflate(&strm, Z_FINISH);
			ZE_ASSERT(ret == Z_STREAM_END && strm.avail_in == 2 && strm.avail_out == 0,
				"Error performing ZLIB inflate decompression!");

			ret = inflateEnd(&strm);
			ZE_ASSERT(ret == Z_OK, "Error ending ZLIB inflate decompression!");
			break;
		}
		case CompressionFormat::Bzip2:
		{
			[[maybe_unused]] S32 ret = BZ2_bzBuffToBuffDecompress(reinterpret_cast<char*>(dst), &dstSize,
				reinterpret_cast<char*>(const_cast<void*>(src)), srcSize - sizeof(U32) - 2, 0, 0);
			ZE_ASSERT(ret == BZ_OK && GetOriginalSize(src, srcSize) == dstSize, "Error performing Bzip2 decompression!");
			break;
		}
		}
	}
}