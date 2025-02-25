#pragma once
#include "Exception/GenericException.h"
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_error.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	// Exception for handling FidelityFX SDK errors
	class FfxException : public Exception::GenericException
	{
		FfxErrorCode code;

	public:
		FfxException(U32 line, const char* file, FfxErrorCode code, std::string&& note) noexcept
			: BasicException(line, file), GenericException(line, file, std::forward<std::string>(note)), code(code) {}
		ZE_CLASS_DEFAULT(FfxException);
		virtual ~FfxException() = default;

		static constexpr const char* TranslateErrorCode(FfxErrorCode code) noexcept;

		constexpr FfxErrorCode GetCode() const noexcept { return code; }
		constexpr const char* GetType() const noexcept override { return "FidelityFX Exception"; }
		constexpr const char* GetErrorString() const noexcept { return TranslateErrorCode(code); }

		const char* what() const noexcept override;
	};

#pragma region Functions
	constexpr const char* FfxException::TranslateErrorCode(FfxErrorCode code) noexcept
	{
		switch (code)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FFX_OK: return "The operation completed successfully.";
		case FFX_ERROR_INVALID_POINTER: return "The operation failed due to an invalid pointer.";
		case FFX_ERROR_INVALID_ALIGNMENT: return "The operation failed due to an invalid alignment.";
		case FFX_ERROR_INVALID_SIZE: return "The operation failed due to an invalid size.";
		case FFX_EOF: return "The end of the file was encountered.";
		case FFX_ERROR_INVALID_PATH: return "The operation failed because the specified path was invalid.";
		case FFX_ERROR_EOF: return "The operation failed because end of file was reached.";
		case FFX_ERROR_MALFORMED_DATA: return "The operation failed because of some malformed data.";
		case FFX_ERROR_OUT_OF_MEMORY: return "The operation failed because it ran out memory.";
		case FFX_ERROR_INCOMPLETE_INTERFACE: return "The operation failed because the interface was not fully configured.";
		case FFX_ERROR_INVALID_ENUM: return "The operation failed because of an invalid enumeration value.";
		case FFX_ERROR_INVALID_ARGUMENT: return "The operation failed because an argument was invalid.";
		case FFX_ERROR_OUT_OF_RANGE: return "The operation failed because a value was out of range.";
		case FFX_ERROR_NULL_DEVICE: return "The operation failed because a device was null.";
		case FFX_ERROR_BACKEND_API_ERROR: return "The operation failed because the backend API returned an error code.";
		case FFX_ERROR_INSUFFICIENT_MEMORY: return "The operation failed because there was not enough memory.";
		case FFX_ERROR_INVALID_VERSION: return "The operation failed because the wrong backend was linked.";
		}
	}
#pragma endregion
}

// Variable holding last result code of FFX SDK call
#define ZE_FFX_EXCEPT_RESULT __ffxErrorCode
// Enable usage of ZE_FFX_* macros in current scope
#define ZE_FFX_ENABLE() [[maybe_unused]] FfxErrorCode ZE_FFX_EXCEPT_RESULT
// Before using needs call to ZE_FFX_ENABLE()
#define ZE_FFX_THROW_FAILED(call, info) if ((ZE_FFX_EXCEPT_RESULT = (call)) != FFX_OK) throw ZE::GFX::FfxException(__LINE__, __FILENAME__, ZE_FFX_EXCEPT_RESULT, info)
// Performs assert check on return value of FFX SDK call, before using needs call to ZE_FFX_ENABLE()
#define ZE_FFX_CHECK(call, info) ZE_FFX_EXCEPT_RESULT = (call); ZE_ASSERT(ZE_FFX_EXCEPT_RESULT == FFX_OK, info)