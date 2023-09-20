#pragma once
#include "CompressionFormat.h"

namespace ZE::IO
{
	// Compression codec that is capable of performing CPU side compression and decompression
	class Compressor final
	{
		CompressionFormat format;

	public:
		constexpr Compressor(CompressionFormat format) noexcept : format(format) { ZE_ASSERT(format != CompressionFormat::None, "Compression codec not needed!"); }
		ZE_CLASS_MOVE(Compressor);
		~Compressor() = default;

		U32 GetOriginalSize(const void* compressedBuffer, U32 compressedSize) const noexcept;
		std::vector<U8> Compress(const void* input, U32 inputSize) const noexcept;
		void Decompress(const void* src, U32 srcSize, void* dst, U32 dstSize) const noexcept;
	};
}