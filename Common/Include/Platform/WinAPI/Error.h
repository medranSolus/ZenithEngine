#pragma once
#include "BasicTypes.h"
#include "WinAPI.h"

namespace ZE::Platform::WinAPI
{
	// Main handler of Windows related errors
	class Error : public std::error_category
	{
	protected:
		Error() = default;

	public:
		ZE_CLASS_MOVE(Error);
		virtual ~Error() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static constexpr Error CATEGORY; return CATEGORY; }
		static Status Make(HRESULT result) noexcept { return { static_cast<int>(result), GetCategory() }; }

		const char* name() const noexcept override { return "WinAPI Error"; }
		std::string message(int condition) const override;
	};
}

#define ZE_WIN_ERROR(hr) ZE::Platform::WinAPI::Error::Make(hr)
// Get last error returned by Windows
#define ZE_WIN_LAST_ERROR() ZE_WIN_ERROR(static_cast<HRESULT>(GetLastError()))
// Check result of the condition and if true, return last error
#define ZE_WIN_RET_FAILED_LAST(condition) do { if (condition) { ZE_BREAK(); return ZE_WIN_LAST_ERROR(); } } while (false)
// Check result of the condition and if true, return last error wrapped in std::unexpected
#define ZE_WIN_RET_FAILED_LAST_EXPECT(condition) do { if (condition) { ZE_BREAK(); return std::unexpected(ZE_WIN_LAST_ERROR()); } } while (false)
// Check result of the call returning HRESULT and if failed, return said error
#define ZE_WIN_RET_FAILED(call) do { HRESULT __hr = (call); if (FAILED(__hr)) { ZE_BREAK(); return ZE_WIN_ERROR(__hr); } } while (false)
// Check result of the call returning HRESULT and if failed, return said error wrapped in std::unexpected
#define ZE_WIN_RET_FAILED_EXPECT(call) do { HRESULT __hr = (call); if (FAILED(__hr)) { ZE_BREAK(); return std::unexpected(ZE_WIN_ERROR(__hr)); } } while (false)