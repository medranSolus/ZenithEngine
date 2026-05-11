#include "IO/Compressor.h"
#include "IO/CompressionError.h"
ZE_WARNING_PUSH
#include "zlib.h"
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

	Expected<std::vector<U8>> Compressor::Compress(const void* input, U32 inputSize) const noexcept
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
			if (deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, 15, 9, Z_DEFAULT_STRATEGY) != Z_OK)
				return std::unexpected(ZE_COMPRESS_ERROR(CompressionResult::ZLIB_InitCompressError));

			// Get max size after decompression
			strm.avail_out = deflateBound(&strm, inputSize);
			compressed.resize(strm.avail_out);
			strm.next_out = compressed.data();

			Status res = {};
			if (deflate(&strm, Z_FINISH) != Z_STREAM_END || strm.avail_in != 0)
				res = ZE_COMPRESS_ERROR(CompressionResult::ZLIB_CompressError);

			if (deflateEnd(&strm) != Z_OK)
			{
				// First preserve previous error if any
				ZE_CODE_RET_FAILED_EXPECT(res);
				res = ZE_COMPRESS_ERROR(CompressionResult::ZLIB_EndCompressError);
			}
			ZE_CODE_RET_FAILED_EXPECT(res);

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
			S32 ret = BZ2_bzBuffToBuffCompress(reinterpret_cast<char*>(compressed.data()), &compressedSize,
				reinterpret_cast<char*>(const_cast<void*>(input)), inputSize, 9, 0, 0);
			if (ret != BZ_OK)
				return std::unexpected(ZE_COMPRESS_ERROR(static_cast<CompressionResult>(ret)));

			// Resize to actual size and append original file size (2 bytes added at the end of stream so there would be no data errors)
			compressed.resize(compressedSize + sizeof(U32) + 2);
			*reinterpret_cast<U32*>(&compressed.at(compressed.size() - sizeof(U32) - 1)) = inputSize;
			break;
		}
		}
		return compressed;
	}

	Status Compressor::Decompress(const void* src, U32 srcSize, void* dst, U32 dstSize) const noexcept
	{
		if (GetOriginalSize(src, srcSize) != dstSize)
			return ZE_COMPRESS_ERROR(CompressionResult::DecompressionSizeMismatch);

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

			if (inflateInit2(&strm, 15) != Z_OK)
				return ZE_COMPRESS_ERROR(CompressionResult::ZLIB_InitDecompressError);

			strm.next_out = reinterpret_cast<U8*>(dst);
			strm.avail_out = dstSize;

			Status res = {};
			if (inflate(&strm, Z_FINISH) != Z_STREAM_END || strm.avail_in != 2 || strm.avail_out != 0)
				res = ZE_COMPRESS_ERROR(CompressionResult::ZLIB_DecompressError);

			if (inflateEnd(&strm) != Z_OK)
			{
				// Preserve previous error if any
				ZE_CODE_RET_FAILED(res);
				res = ZE_COMPRESS_ERROR(CompressionResult::ZLIB_EndDecompressError);
			}
			ZE_CODE_RET_FAILED(res);
			break;
		}
		case CompressionFormat::Bzip2:
		{
			S32 ret = BZ2_bzBuffToBuffDecompress(reinterpret_cast<char*>(dst), &dstSize,
				reinterpret_cast<char*>(const_cast<void*>(src)), srcSize - sizeof(U32) - 2, 0, 0);
			if (ret != BZ_OK)
				return ZE_COMPRESS_ERROR(static_cast<CompressionResult>(ret));
			break;
		}
		}
		return {};
	}
}