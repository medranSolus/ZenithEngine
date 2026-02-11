#pragma once
ZE_WARNING_PUSH
#include "FidelityFX/host/ffx_error.h"
#include "ffx_api.h"
#include "nvsdk_ngx_defs.h"
#include "xess/xess.h"
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

#if _ZE_FFXAPI_ENABLED
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
		static Status Make(NVSDK_NGX_Result code) noexcept { return { static_cast<int>(code), GetCategory() }; }

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
#define ZE_FFX_CHECK(call, info) do { [[maybe_unused]] ffxReturnCode_t __res = (call); ZE_ASSERT(__res == FFX_OK, info); } while (false)

// Get FFX API error status code
#define ZE_FFX_API_ERROR(code) ZE::GFX::Error::FfxApi::Make(code)
// Performs assert check on return value of FFX API call
#define ZE_FFX_API_CHECK(call, info) do { [[maybe_unused]] ffxReturnCode_t __res = (call); ZE_ASSERT(__res == FFX_API_RETURN_OK, info); } while (false)

// Get XeSS error status code
#define ZE_XESS_ERROR(code) ZE::GFX::Error::XeSS::Make(code)
// Performs assert check on return value of XeSS SDK call
#define ZE_XESS_CHECK(call, info) do { [[maybe_unused]] xess_result_t __res = (call); ZE_ASSERT(__res == XESS_RESULT_SUCCESS, info); } while (false)