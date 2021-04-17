#pragma once
#include "Exception/WinApiException.h"
#include "Pixel.h"
#include <functional>
#include <utility>
#include <vector>

// To be changed into multiplatform library
namespace DirectX
{
	typedef Vector XMVECTOR;
}
#include "DirectXTex.h"
namespace Tex
{
	using namespace DirectX;
}

namespace Data
{
	class Surface final
	{
		static constexpr DXGI_FORMAT PIXEL_FORMAT = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

		Tex::ScratchImage scratch;
		const Tex::Image* image;

	public:
		Surface(const std::string& name);
		Surface(U64 width, U64 height, DXGI_FORMAT format = PIXEL_FORMAT);
		Surface(const Surface&) = delete;
		Surface(Surface&&) = default;
		Surface& operator=(const Surface&) = delete;
		Surface& operator=(Surface&&) = default;
		~Surface() = default;

		static bool IsImage(const std::string& ext) noexcept;

		constexpr DXGI_FORMAT GetFormat() const noexcept { return image->format; }
		constexpr U64 GetWidth() const noexcept { return image->width; }
		constexpr U64 GetHeight() const noexcept { return image->height; }
		constexpr U64 GetRowByteSize() const noexcept { return image->rowPitch; }
		constexpr U64 GetSize() const noexcept { return GetWidth() * GetHeight(); }
		constexpr void PutPixel(U64 x, U64 y, const Pixel& c) noexcept;
		constexpr Pixel GetPixel(U64 x, U64 y) const noexcept;

		bool HasAlpha() const noexcept { return !scratch.IsAlphaAllOpaque(); }
		Pixel* GetBuffer() noexcept { return reinterpret_cast<Pixel*>(image->pixels); }
		const Pixel* GetBuffer() const noexcept { return reinterpret_cast<const Pixel*>(image->pixels); }
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