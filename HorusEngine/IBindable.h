#pragma once
#include "IProbeable.h"
#include "WinAPI.h"
#pragma warning(disable:4265)
#include <wrl.h>
#pragma warning(default:4265)
#include <d3d11.h>

namespace GFX::Resource
{
	template<typename T>
	struct is_resolvable_by_codex
	{
		static constexpr bool generate{ false };
	};

	class IBindable : public Probe::IProbeable
	{
		static constexpr const char* NO_CODEX_RID = "?";

	protected:
		static ID3D11DeviceContext* GetContext(Graphics& gfx) noexcept;
		static ID3D11Device* GetDevice(Graphics& gfx) noexcept;

	public:
		IBindable() = default;
		IBindable(const IBindable&) = delete;
		IBindable& operator=(const IBindable&) = delete;
		virtual ~IBindable() = default;

		static constexpr const char* GetNoCodexRID() noexcept { return NO_CODEX_RID; }
		template<typename T, typename ...Params>
		static constexpr std::string GenerateRID(Params&& ...p) noexcept
		{
			static_assert(is_resolvable_by_codex<T>::generate == true, "Class does not implement static method GenerateRID(...)!");
			return T::template GenerateRID(std::forward<Params>(p)...);
		}

		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return false; }

		virtual std::string GetRID() const noexcept = 0;
		virtual void Bind(Graphics& gfx) = 0;
	};
}

#define DEBUG_NAME _name
#ifdef _DEBUG
// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
// Before using needs call to GFX_ENABLE_EXCET()
#define SET_DEBUG_NAME_OWN(child, name) if (child) { GFX_THROW_FAILED(child->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.c_str())); }
#define SET_DEBUG_NAME_SETUP(id) const std::string DEBUG_NAME = id
#else
// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
// Before using needs call to GFX_ENABLE_EXCET()
#define SET_DEBUG_NAME_OWN(child, name)
#define SET_DEBUG_NAME_SETUP(id)
#endif
// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
// Before using needs call to GFX_ENABLE_EXCET()
#define SET_DEBUG_NAME(child, id) SET_DEBUG_NAME_SETUP(id); SET_DEBUG_NAME_OWN(child, DEBUG_NAME)
// Requires "DXGIDebugInfoManager debugInfoManager" available in current scope in DEBUG mode
// Note: Can be obtained from GFX::Graphics via macro GFX_ENABLE_INFO(gfx)
// Before using needs call to GFX_ENABLE_EXCET()
#define SET_DEBUG_NAME_RID(child) SET_DEBUG_NAME(child, GetRID())