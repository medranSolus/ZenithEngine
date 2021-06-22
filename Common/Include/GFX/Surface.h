#pragma once
#include "Pixel.h"
#include "API.h"
#include <functional>
#include <utility>
#include <vector>

namespace ZE::GFX
{
	class Surface final
	{
		static constexpr PixelFormat PIXEL_FORMAT = PixelFormat::R8G8B8A8_UNorm;

		Tex::ScratchImage scratch;
		const Tex::Image* image;

	public:
		Surface(const std::string& name);
		Surface(U64 width, U64 height, PixelFormat format = PIXEL_FORMAT);
		Surface(Surface&&) = default;
		Surface(const Surface&) = delete;
		Surface& operator=(Surface&&) = default;
		Surface& operator=(const Surface&) = delete;
		~Surface() = default;

		static bool IsImage(const std::string& ext) noexcept;

		constexpr PixelFormat GetFormat() const noexcept { return API::GetFormatFromDX(image->format); }
		constexpr U64 GetWidth() const noexcept { return image->width; }
		constexpr U64 GetHeight() const noexcept { return image->height; }
		constexpr U64 GetRowByteSize() const noexcept { return image->rowPitch; }
		constexpr U64 GetSize() const noexcept { return GetWidth() * GetHeight(); }
		constexpr void PutPixel(U64 x, U64 y, const Pixel& c) noexcept;
		constexpr Pixel GetPixel(U64 x, U64 y) const noexcept;

		// NOTE: reinterpret_cast is removed from here for allowing constexpr evaluation
		constexpr Pixel* GetBuffer() noexcept { return (Pixel*)(image->pixels); }
		constexpr const Pixel* GetBuffer() const noexcept { return (const Pixel*)(image->pixels); }

		bool HasAlpha() const noexcept { return !scratch.IsAlphaAllOpaque(); }
		void Clear(const Pixel& color) noexcept { memset(GetBuffer(), color, GetSize() * sizeof(Pixel)); }

		void Save(const std::string& filename) const;
	};

#pragma region Functions
	constexpr void Surface::PutPixel(U64 x, U64 y, const Pixel& c) noexcept
	{
		assert(x >= 0);
		assert(y >= 0);
		assert(x < GetWidth());
		assert(y < GetHeight());
		GetBuffer()[y * GetWidth() + x] = c;
	}

	constexpr Pixel Surface::GetPixel(U64 x, U64 y) const noexcept
	{
		assert(x >= 0);
		assert(y >= 0);
		assert(x < GetWidth());
		assert(y < GetHeight());
		return GetBuffer()[y * GetWidth() + x];
	}
#pragma endregion
}