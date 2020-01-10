#pragma once
#include "BasicException.h"
#include <memory>

namespace GFX
{
	class Surface
	{
	public:
		class Pixel
		{
		public:
			// ARGB
			unsigned int dword = 255 << 24U;

			Pixel() = default;
			constexpr Pixel(const Pixel & p) noexcept : dword(p.dword) {}
			constexpr Pixel(unsigned int dw) noexcept : dword(dw) {}
			constexpr Pixel(unsigned char r, unsigned char g, unsigned char b) noexcept : dword((r << 16U) | (g << 8U) | b) {}
			constexpr Pixel(unsigned char a, unsigned char r, unsigned char g, unsigned char b) noexcept : dword((a << 24U) | (r << 16U) | (g << 8U) | b) {}
			constexpr Pixel(Pixel col, unsigned char a) noexcept : Pixel((a << 24U) | col.dword) {}
			constexpr Pixel & operator=(const Pixel & color) noexcept { dword = color.dword; return *this; }

			constexpr unsigned char GetA() const noexcept { return dword >> 24U; }
			constexpr unsigned char GetR() const noexcept { return (dword >> 16U) & 0xFFU; }
			constexpr unsigned char GetG() const noexcept { return (dword >> 8U) & 0xFFU; }
			constexpr unsigned char GetB() const noexcept { return dword & 0xFFU; }

			constexpr void SetA(unsigned char x) noexcept { dword = (dword & 0x00FFFFFFU) | (x << 24U); }
			constexpr void SetR(unsigned char r) noexcept { dword = (dword & 0xFF00FFFFU) | (r << 16U); }
			constexpr void SetG(unsigned char g) noexcept { dword = (dword & 0xFFFF00FFU) | (g << 8U); }
			constexpr void SetB(unsigned char b) noexcept { dword = (dword & 0xFFFFFF00U) | b; }
		};

	private:
		std::unique_ptr<Pixel[]> buffer;
		unsigned int width;
		unsigned int height;

		inline Surface(unsigned int width, unsigned int height, std::unique_ptr<Pixel[]> bufferParam) noexcept 
			: buffer(std::move(bufferParam)), width(width), height(height) {}

	public:
		Surface(const std::string & name);
		Surface(unsigned int width, unsigned int height) noexcept : buffer(std::make_unique<Pixel[]>(width * height)), width(width), height(height) {}
		Surface(Surface && surface) noexcept : buffer(std::move(surface.buffer)), width(surface.width), height(surface.height) {}
		Surface(const Surface &) = delete;
		Surface & operator=(Surface && surface) noexcept;
		Surface & operator=(const Surface &) = delete;
		~Surface() = default;

		constexpr unsigned int GetWidth() const noexcept { return width; }
		constexpr unsigned int GetHeight() const noexcept { return height; }
		inline Pixel * GetBuffer() noexcept { return buffer.get(); }
		inline const Pixel * GetBuffer() const noexcept { return buffer.get(); }
		inline void Clear(const Pixel & value) noexcept { memset(buffer.get(), value.dword, width * height * sizeof(Pixel)); }

		constexpr void PutPixel(unsigned int x, unsigned int y, Pixel c) noexcept(!IS_DEBUG);
		constexpr Pixel GetPixel(unsigned int x, unsigned int y) const noexcept(!IS_DEBUG);
		inline void Copy(const Surface & surface) noexcept(!IS_DEBUG);

		void Save(const std::string & filename) const;
		
		class ImageException : public Exception::BasicException
		{
			std::string info;

		public:
			ImageException(unsigned int line, const char* file, std::string note) noexcept
				: BasicException(line, file), info(std::move(note)) {}

			inline const char * GetType() const noexcept override { return "Image Exception"; }
			constexpr const std::string & GetImageInfo() const noexcept { return info; }

			const char* what() const noexcept override;
		};
	};
}
