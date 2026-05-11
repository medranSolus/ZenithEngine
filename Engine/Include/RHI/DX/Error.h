#pragma once
#include "Platform/WinAPI/Error.h"
#include "DebugInfoManager.h"

// Make custom DXGI based HRESULT error code
#define ZE_MAKE_DXGI_ERROR(u16) (0xA7FF0000L | (0xFFFF & u16))

namespace ZE::RHI::DX
{
	// Main handler for DirectX Graphics Infrastructure based APIs error
	class Error : public Platform::WinAPI::Error
	{
	protected:
		Error() = default;

	public:
		static constexpr HRESULT DEBUG_MSG_ERROR = ZE_MAKE_DXGI_ERROR(1);
		static constexpr HRESULT ALLOC_ERROR = ZE_MAKE_DXGI_ERROR(2);
		static constexpr HRESULT NO_ADAPTER_ERROR = ZE_MAKE_DXGI_ERROR(3);
		static constexpr HRESULT ENHANCED_BARRIERS_ERROR = ZE_MAKE_DXGI_ERROR(4);
		static constexpr HRESULT DSTORAGE_REQUEST_FAILURE = ZE_MAKE_DXGI_ERROR(5);
		static constexpr HRESULT DISKMANAGER_INVALID_HANDLE = ZE_MAKE_DXGI_ERROR(6);

		ZE_CLASS_MOVE(Error);
		virtual ~Error() = default;
		
		static constexpr const std::error_category& GetCategory() noexcept { static constexpr Error CATEGORY; return CATEGORY; }
		static Status Make(HRESULT result) noexcept { return { static_cast<int>(result), GetCategory() }; }

		const char* name() const noexcept override { return "DirectX Graphics Error"; }
		std::string message(int condition) const override;
	};
}

#if _ZE_DEBUG_GFX_API
// Check if call produced any related DirectX error messages
#	define ZE_DX_CHECK_FAILED(call, msg) do { (call); if (ZE::RHI::DX::DebugInfoManager::CheckMessages()) { ZE_CODE_ERROR(ZE::RHI::DX::Error::Make(ZE::RHI::DX::Error::DEBUG_MSG_ERROR), msg); } } while (false)
// Check if call produced any related DirectX error messages and return error on them
#	define ZE_DX_RET_FAILED_DEBUG(call) do { (call); if (ZE::RHI::DX::DebugInfoManager::CheckMessages()) { return ZE::RHI::DX::Error::Make(ZE::RHI::DX::Error::DEBUG_MSG_ERROR); } } while (false)
// Check if call produced any related DirectX error messages and return error on them, wrapped in std::unexpected
#	define ZE_DX_RET_FAILED_DEBUG_EXPECT(call) do { (call); if (ZE::RHI::DX::DebugInfoManager::CheckMessages()) { return std::unexpected(ZE::RHI::DX::Error::Make(ZE::RHI::DX::Error::DEBUG_MSG_ERROR)); } } while (false)
// Set HRESULT variable to appriopriate error if DirectX debug layer found any messages
#	define ZE_DX_CHECK_DEBUG_INFO(hr) if (ZE::RHI::DX::DebugInfoManager::CheckMessages()) hr = FAILED(hr) ? hr : ZE::RHI::DX::Error::DEBUG_MSG_ERROR
#else
// Check if call produced any related DirectX error messages
#	define ZE_DX_CHECK_FAILED(call) (call)
// Check if call produced any related DirectX error messages and return error on them
#	define ZE_DX_RET_FAILED_DEBUG(call) (call)
// Check if call produced any related DirectX error messages and return error on them, wrapped in std::unexpected
#	define ZE_DX_RET_FAILED_DEBUG_EXPECT(call) (call)
// Set HRESULT variable to appriopriate error if DirectX debug layer found any messages
#	define ZE_DX_CHECK_DEBUG_INFO(hr) ((void)0)
#endif

// Check result of the call returning HRESULT and if failed, return said error
#define ZE_DX_RET_FAILED(call) do { HRESULT __hr = (call); ZE_DX_CHECK_DEBUG_INFO(__hr); if (FAILED(__hr)) { ZE_BREAK(); return ZE_WIN_ERROR(__hr); } } while (false)
// Check result of the call returning HRESULT and if failed, return said error wrapped in std::unexpected
#define ZE_DX_RET_FAILED_EXPECT(call) do { HRESULT __hr = (call); ZE_DX_CHECK_DEBUG_INFO(__hr); if (FAILED(__hr)) { ZE_BREAK(); return std::unexpected(ZE_WIN_ERROR(__hr)); } } while (false)

#if _ZE_DEBUG_GFX_NAMES
// Sets debug name for GPU object with given id
#	define ZE_DX_SET_ID(child, id) do { std::string __debugID = id; HRESULT __hr = child->SetPrivateData(WKPDID_D3DDebugObjectName, Utils::SafeCast<UINT>(__debugID.size()), __debugID.c_str()); if (FAILED(__hr)) { ZE_CODE_WARNING(ZE::RHI::DX::Error::Make(__hr), "Failed to set debug name! Name" + __debugID); } } while (false)
#else
// Sets debug name for GPU object with given id
#	define ZE_DX_SET_ID(child, id) ((void)0)
#endif