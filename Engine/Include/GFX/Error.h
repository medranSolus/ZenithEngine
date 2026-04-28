#pragma once
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_error.h"
#if _ZE_FFX_API_ENABLED
#	include "ffx_api.h"
#endif
#if _ZE_NGX_ENABLED
#	include "nvsdk_ngx_defs.h"
#endif
#if _ZE_XESS_ENABLED
#	include "xess/xess.h"
#endif
ZE_WARNING_POP

namespace ZE::GFX::Error
{
	// Error handling for FidelityFX SDK codes
	class Ffx : public std::error_category
	{
	protected:
		Ffx() = default;

	public:
		ZE_CLASS_MOVE(Ffx);
		virtual ~Ffx() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static Ffx CATEGORY; return CATEGORY; }
		static Status Make(FfxErrorCode code) noexcept { return { static_cast<int>(code), GetCategory() }; }

		const char* name() const noexcept override { return "FidelityFX Error"; }
		std::string message(int condition) const override;
	};

#if _ZE_FFX_API_ENABLED
	// Error handling for FidelityFX API codes
	class FfxApi : public std::error_category
	{
	protected:
		FfxApi() = default;

	public:
		ZE_CLASS_MOVE(FfxApi);
		virtual ~FfxApi() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static FfxApi CATEGORY; return CATEGORY; }
		static Status Make(ffxReturnCode_t code) noexcept { return { static_cast<int>(code), GetCategory() }; }

		const char* name() const noexcept override { return "FidelityFX API Error"; }
		std::string message(int condition) const override;
	};
#endif

#if _ZE_NGX_ENABLED
	// Error handling for NGX results
	class NGX : public std::error_category
	{
	protected:
		NGX() = default;

	public:
		ZE_CLASS_MOVE(NGX);
		virtual ~NGX() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static NGX CATEGORY; return CATEGORY; }
		static Status Make(NVSDK_NGX_Result code) noexcept { return { code == NVSDK_NGX_Result_Success ? 0 : static_cast<int>(code), GetCategory() }; }

		const char* name() const noexcept override { return "NVSDK NGX Error"; }
		std::string message(int condition) const override;
	};
#endif

#if _ZE_XESS_ENABLED
	// Error handling for XeSS SDK results
	class XeSS : public std::error_category
	{
	protected:
		XeSS() = default;

	public:
		ZE_CLASS_MOVE(XeSS);
		virtual ~XeSS() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static XeSS CATEGORY; return CATEGORY; }
		static Status Make(xess_result_t code) noexcept { return { static_cast<int>(code), GetCategory() }; }

		const char* name() const noexcept override { return "XeSS Error"; }
		std::string message(int condition) const override;
	};
#endif
}

// Get FFX SDK error status code
#define ZE_FFX_ERROR(code) ZE::GFX::Error::Ffx::Make(code)
// Performs assert check on return value of FFX SDK call
#define ZE_FFX_CHECK(call, info) do { [[maybe_unused]] FfxErrorCode __res = (call); ZE_ASSERT(__res == FFX_OK, info); } while (false)
// Return FFX SDK error if call failed and log error message
#define ZE_FFX_LOG_RET_FAILED(call, info) do { FfxErrorCode __res = (call); if (__res != FFX_OK) { Status __ffxStatus = ZE_FFX_ERROR(__res); ZE_CODE_ERROR(__ffxStatus, info); return __ffxStatus; } } while (false)
// Return FFX SDK error if call failed (wrapped in std::unexpected) and log error message
#define ZE_FFX_LOG_RET_FAILED_EXPECT(call, info) do { FfxErrorCode __res = (call); if (__res != FFX_OK) { Status __ffxStatus = ZE_FFX_ERROR(__res); ZE_CODE_ERROR(__ffxStatus, info); return std::unexpected(__ffxStatus); } } while (false)

// Get FFX API error status code
#define ZE_FFX_API_ERROR(code) ZE::GFX::Error::FfxApi::Make(code)
// Performs assert check on return value of FFX API call
#define ZE_FFX_API_CHECK(call, info) do { [[maybe_unused]] ffxReturnCode_t __res = (call); ZE_ASSERT(__res == FFX_API_RETURN_OK, info); } while (false)
// Return FFX API error if call failed and log error message
#define ZE_FFX_API_LOG_RET_FAILED(call, info) do { ffxReturnCode_t __res = (call); if (__res != FFX_API_RETURN_OK) { Status __ffxStatus = ZE_FFX_API_ERROR(__res); ZE_CODE_ERROR(__ffxStatus, info); return __ffxStatus; } } while (false)
// Return FFX API error if call failed (wrapped in std::unexpected) and log error message
#define ZE_FFX_API_LOG_RET_FAILED_EXPECT(call, info) do { ffxReturnCode_t __res = (call); if (__res != FFX_API_RETURN_OK) { Status __ffxStatus = ZE_FFX_API_ERROR(__res); ZE_CODE_ERROR(__ffxStatus, info); return std::unexpected(__ffxStatus); } } while (false)

// Get NGX SDK error status code
#define ZE_NGX_ERROR(code) ZE::GFX::Error::NGX::Make(code)
// Performs assert check on return value of NGX call
#define ZE_NGX_CHECK(call, info) do { [[maybe_unused]] NVSDK_NGX_Result __res = (call); ZE_ASSERT(NVSDK_NGX_SUCCEED(__res), info); } while (false)
// Return NGX SDK error if call failed and log error message
#define ZE_NGX_LOG_RET_FAILED(call, info) do { Status __ngxStatus  = (call); if (__ngxStatus) { ZE_CODE_ERROR(__ngxStatus, info); return __ngxStatus; } } while (false)
// Return NGX SDK error if call failed (wrapped in std::unexpected) and log error message
#define ZE_NGX_LOG_RET_FAILED_EXPECT(call, info) do { Status __ngxStatus = (call); if (__ngxStatus) { ZE_CODE_ERROR(__ngxStatus, info); return std::unexpected(__ngxStatus); } } while (false)

// Get XeSS error status code
#define ZE_XESS_ERROR(code) ZE::GFX::Error::XeSS::Make(code)
// Performs assert check on return value of XeSS call
#define ZE_XESS_CHECK(call, info) do { [[maybe_unused]] xess_result_t __res = (call); ZE_ASSERT(__res < XESS_RESULT_SUCCESS, info); } while (false)
// Return XeSS error if call failed and log error message
#define ZE_XESS_LOG_RET_FAILED(call, info) do { xess_result_t __res = (call); if (__res < XESS_RESULT_SUCCESS) { Status __xessStatus = ZE_XESS_ERROR(__res); ZE_CODE_ERROR(__xessStatus, info); return __xessStatus; } } while (false)
// Return XeSS error if call failed (wrapped in std::unexpected) and log error message
#define ZE_XESS_LOG_RET_FAILED_EXPECT(call, info) do { xess_result_t __res = (call); if (__res < XESS_RESULT_SUCCESS) { Status __xessStatus = ZE_XESS_ERROR(__res); ZE_CODE_ERROR(__xessStatus, info); return std::unexpected(__xessStatus); } } while (false)