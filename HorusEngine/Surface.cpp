#include "Surface.h"
#define USE_WINDOWS_DEFINES
#include "WinAPI.h"
#include "Utils.h"
#include <gdiplus.h>
#include <fstream>
#include <cassert>

#define IMG_THROW_EXCEPT(info) throw ImageException(__LINE__, __FILE__, info)

namespace GFX
{
	Surface::Surface(const std::string & name)
	{
		Gdiplus::Bitmap bitmap(toUtf8(name).c_str());
		if (bitmap.GetLastStatus() != Gdiplus::Status::Ok)
			IMG_THROW_EXCEPT("Loading image \"" + name + "\": failed.");

		width = bitmap.GetWidth();
		height = bitmap.GetHeight();
		buffer = std::make_unique<Pixel[]>(width * height);
		for (unsigned int y = 0; y < height; ++y)
		{
			for (unsigned int x = 0; x < width; ++x)
			{
				Gdiplus::Color c;
				bitmap.GetPixel(x, y, &c);
				buffer[y * width + x] = c.GetValue();
			}
		}
	}

	Surface & Surface::operator=(Surface && surface) noexcept
	{
		width = surface.width;
		height = surface.height;
		buffer = std::move(surface.buffer);
		surface.buffer = nullptr;
		return *this;
	}
	
	constexpr void Surface::PutPixel(unsigned int x, unsigned int y, Pixel c) noexcept(!IS_DEBUG)
	{
		assert(x >= 0);
		assert(y >= 0);
		assert(x < width);
		assert(y < height);
		buffer[y * width + x] = c;
	}

	constexpr Surface::Pixel Surface::GetPixel(unsigned int x, unsigned int y) const noexcept(!IS_DEBUG)
	{
		assert(x >= 0);
		assert(y >= 0);
		assert(x < width);
		assert(y < height);
		return buffer[y * width + x];
	}

	inline void Surface::Copy(const Surface& surface) noexcept(!IS_DEBUG)
	{
		assert(width >= surface.width);
		assert(height >= surface.height);
		memcpy(buffer.get(), surface.buffer.get(), surface.width * surface.height * sizeof(Pixel));
	}

	void Surface::Save(const std::string& filename) const
	{
		// https://docs.microsoft.com/en-gb/windows/win32/gdiplus/-gdiplus-retrieving-the-class-identifier-for-an-encoder-use?redirectedfrom=MSDN

		CLSID clsId;
		const WCHAR * format = L"image/png";
		UINT num = 0;  // number of image encoders
		UINT size = 0; // size of the image encoder array in bytes
		Gdiplus::ImageCodecInfo * imgCodecInfo = nullptr;
		Gdiplus::GetImageEncodersSize(&num, &size);
		if (size == 0)
			IMG_THROW_EXCEPT("Saving surface to \"" + filename + "\": cannot get encoder size.");
		imgCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
		if (imgCodecInfo == nullptr)
			IMG_THROW_EXCEPT("Saving surface to \"" + filename + "\": cannot allocate memory.");

		GetImageEncoders(num, size, imgCodecInfo);
		for (UINT i = 0; i < num; ++i)
		{
			if (wcscmp(imgCodecInfo[i].MimeType, format) == 0)
			{
				clsId = imgCodecInfo[i].Clsid;
				free(imgCodecInfo);
				Gdiplus::Bitmap bitmap(width, height, width * sizeof(Pixel), PixelFormat32bppARGB, (BYTE*)buffer.get());
				if (bitmap.Save(boost::locale::conv::to_utf<wchar_t>(filename, "UTF-8").c_str(), &clsId, nullptr) != Gdiplus::Status::Ok)
					IMG_THROW_EXCEPT("Saving surface to \"" + filename + "\": failed to save.");
				return;
			}
		}
		free(imgCodecInfo);
		IMG_THROW_EXCEPT("Saving surface to \"" + filename + "\": cannot find matching encoder <" +
			boost::locale::conv::from_utf(format, "UTF-8") + ">.");
	}
	
	const char * Surface::ImageException::what() const noexcept
	{
		std::ostringstream stream;
		stream << this->BasicException::what()
			<< "\n[Image Info] " << GetImageInfo();
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}
