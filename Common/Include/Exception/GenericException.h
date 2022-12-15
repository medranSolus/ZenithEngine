#pragma once
#include "BasicException.h"

namespace ZE::Exception
{
	// Generic exception for simple message
	class GenericException : public virtual BasicException
	{
		const char* type;
		std::string info;

	public:
		GenericException(U32 line, const char* file, std::string&& note, const char* type = nullptr) noexcept
			: BasicException(line, file), type(type), info(std::move(note)) {}
		ZE_CLASS_DEFAULT(GenericException);
		virtual ~GenericException() = default;

		constexpr const std::string& GetInfo() const noexcept { return info; }
		constexpr const char* GetType() const noexcept override { return type; }

		const char* what() const noexcept override;
	};
}

// Exception thrown while processing images
#define ZE_IMG_EXCEPT(info) ZE::Exception::GenericException(__LINE__, __FILENAME__, info, "Image Exception")
// Exception thrown when passed incorrect params
#define ZE_ARG_EXCEPT(info) ZE::Exception::GenericException(__LINE__, __FILENAME__, info, "Wrong Argument Exception")
// Exception thrown while creating render graph
#define ZE_RGC_EXCEPT(info) ZE::Exception::GenericException(__LINE__, __FILENAME__, info, "Render Graph Compile Exception")
// Exception thrown while loading models
#define ZE_MDL_EXCEPT(info) ZE::Exception::GenericException(__LINE__, __FILENAME__, info, "Model Loading Exception")
// Exception thrown when checking versions
#define ZE_CMP_EXCEPT(info) ZE::Exception::GenericException(__LINE__, __FILENAME__, info, "Compatibility Exception")
// Exception thrown when doing generic IO
#define ZE_IO_EXCEPT(info) ZE::Exception::GenericException(__LINE__, __FILENAME__, info, "I/O Exception")