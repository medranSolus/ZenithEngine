#pragma once
#include "BasicException.h"

namespace ZE::Exception
{
	class ImageException : public virtual BasicException
	{
		std::string info;

	public:
		ImageException(U32 line, const char* file, std::string&& note) noexcept
			: BasicException(line, file), info(std::move(note)) {}
		ImageException(ImageException&&) = default;
		ImageException(const ImageException&) = default;
		ImageException& operator=(ImageException&&) = default;
		ImageException& operator=(const ImageException&) = default;
		virtual ~ImageException() = default;

		constexpr const std::string& GetImageInfo() const noexcept { return info; }
		constexpr const char* GetType() const noexcept override { return "Image Exception"; }

		const char* what() const noexcept override;
	};
}

#define ZE_IMG_EXCEPT(info) ZE::Exception::ImageException(__LINE__, __FILENAME__, info)