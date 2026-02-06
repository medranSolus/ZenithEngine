#pragma once
#include "BasicTypes.h"
#include <string>

namespace ZE::IO::DDS
{
	// Result of DDS file operations
	enum class FileResult : U8
	{
		Ok, IncorrectMagicNumber, UnknownFormat, MissingCubemapFaces, IllformattedVolumeTexture,
		IncorrectArraySize, Incorrect1DTextureHeight, IncorrectDimension, Unknown
	};

	// Main handler of DDS related errors
	class Error : public std::error_category
	{
	protected:
		Error() = default;

	public:
		ZE_CLASS_MOVE(Error);
		virtual ~Error() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static Error CATEGORY; return CATEGORY; }
		static Status Make(FileResult result) noexcept { return { static_cast<int>(result), GetCategory() }; }

		const char* name() const noexcept override { return "DDS Error"; }
		std::string message(int condition) const override;
	};
}