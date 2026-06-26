#pragma once
#include "Platform/WinAPI/Error.h"

namespace ZE::AHI::XAudio2
{
	// Main handler for XAudio2 based APIs error
	class Error : public Platform::WinAPI::Error
	{
	protected:
		Error() = default;

	public:
		ZE_CLASS_MOVE(Error);
		virtual ~Error() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static constexpr Error CATEGORY; return CATEGORY; }
		static Status Make(HRESULT result) noexcept { return { static_cast<int>(result), GetCategory() }; }

		const char* name() const noexcept override { return "XAudio2 Error"; }
	};
}

// Check result of the call returning HRESULT and if failed return said error
#define ZE_XA2_RET_FAILED(call) do { HRESULT __hr = (call); if (FAILED(__hr)) { ZE_BREAK(); return ZE::AHI::XAudio2::Error::Make(__hr); } } while (false)
// Check result of the call returning HRESULT and if failed return said error wrapped in std::unexpected
#define ZE_XA2_RET_FAILED_EXPECT(call) do { HRESULT __hr = (call); if (FAILED(__hr)) { ZE_BREAK(); return std::unexpected(ZE::AHI::XAudio2::Error::Make(__hr)); } } while (false)
