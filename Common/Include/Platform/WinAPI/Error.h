#pragma once
#include "BasicTypes.h"
#include "WinAPI.h"

namespace ZE::Platform::WinAPI
{
	// Main handler of Windows related errors
	class Error : public std::error_category
	{
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

// Get last error returned by Windows
#define	ZE_WIN_LAST_ERROR() ZE::Platform::WinAPI::Error::Make(static_cast<HRESULT>(GetLastError()))