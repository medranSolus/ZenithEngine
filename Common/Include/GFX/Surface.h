#pragma once
#include "Pixel.h"
#include "DX.h"
#include <utility>
#include <vector>

namespace ZE::GFX
{
	// Texture raw data
	class Surface final
	{
		static constexpr PixelFormat PIXEL_FORMAT = PixelFormat::R8G8B8A8_UNorm;

		Tex::ScratchImage scratch;
		Ptr<const Tex::Image> image;

	public:
		Surface(const std::string& name);
		Surface(U64 width, U64 height, PixelFormat format = PIXEL_FORMAT);
		ZE_CLASS_MOVE(Surface);
		~Surface() = default;

		static bool IsImage(const std::string& ext) noexcept;

		constexpr PixelFormat GetFormat() const noexcept { return API::DX::GetFormatFromDX(image->format); }
		constexpr U64 GetWidth() const noexcept { return image->width; }
		constexpr U64 GetHeight() const noexcept { return image->height; }
		constexpr U64 GetRowByteSize() const noexcept { return image->rowPitch; }
		constexpr U64 GetPixelSize() const noexcept { return GetRowByteSize() / GetWidth(); }
		constexpr U64 GetSliceByteSize() const noexcept { return image->slicePitch; }
		constexpr U64 GetSize() const noexcept { return GetWidth() * GetHeight(); }
		constexpr void PutPixel(U64 x, U64 y, const Pixel& c) noexcept;
		constexpr Pixel GetPixel(U64 x, U64 y) const noexcept;

		// NOTE: reinterpret_cast is removed from here for allowing constexpr evaluation
		constexpr Pixel* GetBuffer() noexcept { return (Pixel*)image->pixels; }
		constexpr const Pixel* GetBuffer() const noexcept { return (const Pixel*)image->pixels; }

		bool HasAlpha() const noexcept { return !scratch.IsAlphaAllOpaque(); }
		void Clear(const Pixel& color) noexcept { memset(GetBuffer(), static_cast<int>(color), GetSize() * sizeof(Pixel)); }

		void Save(const std::string& filename) const;
	};

#pragma region Functions
	constexpr void Surface::PutPixel(U64 x, U64 y, const Pixel& c) noexcept
	{
		ZE_ASSERT(x < GetWidth(), "X out of bounds!");
		ZE_ASSERT(y < GetHeight(), "Y out of bounds!");
		GetBuffer()[y * GetWidth() + x] = c;
	}

	constexpr Pixel Surface::GetPixel(U64 x, U64 y) const noexcept
	{
		ZE_ASSERT(x < GetWidth(), "X out of bounds!");
		ZE_ASSERT(y < GetHeight(), "Y out of bounds!");
		return GetBuffer()[y * GetWidth() + x];
	}
#pragma endregion
}