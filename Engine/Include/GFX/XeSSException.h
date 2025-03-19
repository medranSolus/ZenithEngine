#pragma once
#include "Exception/GenericException.h"
ZE_WARNING_PUSH
#include "xess/xess.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	// Exception for handling XeSS SDK errors
	class XeSSException : public Exception::GenericException
	{
		xess_result_t code;

	public:
		XeSSException(U32 line, const char* file, xess_result_t code, std::string&& note) noexcept
			: BasicException(line, file), GenericException(line, file, std::forward<std::string>(note)), code(code) {}
		ZE_CLASS_DEFAULT(XeSSException);
		virtual ~XeSSException() = default;

		static constexpr const char* TranslateErrorCode(xess_result_t code) noexcept;

		constexpr xess_result_t GetCode() const noexcept { return code; }
		constexpr const char* GetType() const noexcept override { return "XeSS Exception"; }
		constexpr const char* GetErrorString() const noexcept { return TranslateErrorCode(code); }

		const char* what() const noexcept override;
	};

#pragma region Functions
	constexpr const char* XeSSException::TranslateErrorCode(xess_result_t code) noexcept
	{
		switch (code)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case XESS_RESULT_WARNING_NONEXISTING_FOLDER: return "Warning. Folder to store dump data doesn't exist. Write operation skipped.";
		case XESS_RESULT_WARNING_OLD_DRIVER: return "An old or outdated driver.";
		case XESS_RESULT_SUCCESS: return "XeSS operation was successful.";
		case XESS_RESULT_ERROR_UNSUPPORTED_DEVICE: return "XeSS not supported on the GPU. An Shader Model 6.4 capable GPU is required.";
		case XESS_RESULT_ERROR_UNSUPPORTED_DRIVER: return "An unsupported driver.";
		case XESS_RESULT_ERROR_UNINITIALIZED: return "Execute called without initialization.";
		case XESS_RESULT_ERROR_INVALID_ARGUMENT: return "Invalid argument such as descriptor handles.";
		case XESS_RESULT_ERROR_DEVICE_OUT_OF_MEMORY: return "Not enough available GPU memory.";
		case XESS_RESULT_ERROR_DEVICE: return "Device function such as resource or descriptor creation.";
		case XESS_RESULT_ERROR_NOT_IMPLEMENTED: return "The function is not implemented.";
		case XESS_RESULT_ERROR_INVALID_CONTEXT: return "Invalid context.";
		case XESS_RESULT_ERROR_OPERATION_IN_PROGRESS: return "Operation not finished yet.";
		case XESS_RESULT_ERROR_UNSUPPORTED: return "Operation not supported in current configuration.";
		case XESS_RESULT_ERROR_CANT_LOAD_LIBRARY: return "The library cannot be loaded.";
		case XESS_RESULT_ERROR_UNKNOWN: return "Unknown internal failure.";
		}
	}
#pragma endregion
}

// Variable holding last result code of XeSS SDK call
#define ZE_XESS_EXCEPT_RESULT __xessErrorCode
// Enable usage of ZE_XESS_* macros in current scope
#define ZE_XESS_ENABLE() [[maybe_unused]] xess_result_t ZE_XESS_EXCEPT_RESULT = XESS_RESULT_SUCCESS
// Before using needs call to ZE_XESS_ENABLE()
#define ZE_XESS_THROW_FAILED(call, info) if ((ZE_XESS_EXCEPT_RESULT = (call)) != XESS_RESULT_SUCCESS) throw ZE::GFX::XeSSException(__LINE__, __FILENAME__, ZE_XESS_EXCEPT_RESULT, info)
// Performs assert check on return value of XeSS SDK call, before using needs call to ZE_XESS_ENABLE()
#define ZE_XESS_CHECK(call, info) ZE_XESS_EXCEPT_RESULT = (call); ZE_ASSERT(ZE_XESS_EXCEPT_RESULT == XESS_RESULT_SUCCESS, info)