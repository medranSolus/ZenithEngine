#pragma once
#include "PixelFormat.h"
#include <filesystem>
#include <utility>
#include <vector>

namespace ZE::GFX
{
	// Texture raw data
	class Surface final
	{
	public:
		// Minimal alignment that texture rows must meet, defined per platform that assets are created for
		static constexpr U32 ROW_PITCH_ALIGNMENT = 256U;
		static constexpr U64 SLICE_PITCH_ALIGNMENT = 512U;

	private:
		enum class CopySource : bool { GrayscaleAlpha, RGB };

		PixelFormat format = PixelFormat::Unknown;
		bool alpha = false;
		U32 width = 0;
		U32 height = 0;
		U16 depth = 0;
		U16 mipCount = 0;
		U16 arraySize = 0;
		U64 memorySize = 0;
		std::shared_ptr<U8[]> memory = nullptr;

		template<CopySource SRC_FORMAT, typename T>
		static void CopyLoadedImage(T* destImage, const T* srcImage, U32 width, U32 height, U32 destRowSize) noexcept;

	public:
		Surface() = default;
		Surface(U32 width, U32 height, PixelFormat format = PixelFormat::R8G8B8A8_UNorm, const void* srcImage = nullptr) noexcept;
		Surface(U32 width, U32 height, U16 depth, U16 mipCount, U16 arraySize, PixelFormat format, bool alpha, const void* srcImage = nullptr) noexcept;
		ZE_CLASS_MOVE(Surface);
		~Surface() = default;

		static constexpr U32 GetRowByteSize(U32 width, PixelFormat format, U16 mip) noexcept { return Math::AlignUp((std::max(width >> mip, 1U) * Utils::GetFormatBitCount(format)) / 8, ROW_PITCH_ALIGNMENT); }
		static constexpr U64 GetSliceByteSize(U32 width, U32 height, PixelFormat format, U16 mip) noexcept { return Math::AlignUp(static_cast<U64>(GetRowByteSize(width, format, mip)) * std::max(height >> mip, 1U), SLICE_PITCH_ALIGNMENT); }

		constexpr PixelFormat GetFormat() const noexcept { return format; }
		constexpr bool HasAlpha() const noexcept { return alpha; }
		constexpr U32 GetWidth() const noexcept { return width; }
		constexpr U32 GetHeight() const noexcept { return height; }
		constexpr U16 GetDepth() const noexcept { return depth; }
		constexpr U16 GetMipCount() const noexcept { return mipCount; }
		constexpr U16 GetArraySize() const noexcept { return arraySize; }
		constexpr U32 GetRowByteSize(U16 mip = 0) const noexcept { return GetRowByteSize(width, format, mip); }
		constexpr U64 GetSliceByteSize(U16 mip = 0) const noexcept { return GetSliceByteSize(width, height, format, mip); }
		constexpr U64 GetMemorySize() const noexcept { return memorySize; }
		constexpr U8 GetPixelSize() const noexcept { return Utils::GetFormatBitCount(format) / 8; }

		std::shared_ptr<U8[]> GetMemory() noexcept { return memory; }
		std::shared_ptr<const U8[]> GetMemory() const noexcept { return memory; }
		U8* GetBuffer() noexcept { return memory.get(); }
		const U8* GetBuffer() const noexcept { return memory.get(); }

		bool Load(std::string_view filename, bool forceAlphaCheck = false, bool allocMips = false) noexcept;
		bool Save(std::string_view filename) const noexcept;
		U8* GetImage(U16 arrayIndex, U16 mipIndex, U16 depthLevel) noexcept;
		bool ExtractChannel(Surface* channelR, Surface* channelG, Surface* channelB, Surface* channelA) const noexcept;
	};
}