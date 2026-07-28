#pragma once
#if _ZE_FFX_API_ENABLED
#	include "FfxApiInterface.h"
#endif
#if _ZE_NGX_ENABLED
#	include "NgxInterface.h"
#endif
#if _ZE_XESS_ENABLED
#	include "XeSSInterface.h"
#endif

namespace ZE::GFX::External
{
	// Static class for accessing external SDKs and APIs related to graphics features, such as FidelityFX, NGX or XeSS.
	// It also serves as a single point of initialization for those SDKs and APIs.
	class InterfaceStorage final
	{
#if _ZE_FFX_API_ENABLED
		static inline std::unique_ptr<FfxApiInterface> ffx;
		static inline U8 ffxRefCount = 0;
#endif
#if _ZE_NGX_ENABLED
		static inline std::unique_ptr<NgxInterface> ngx;
		static inline U8 ngxRefCount = 0;
#endif
#if _ZE_XESS_ENABLED
		static inline std::unique_ptr<XeSSInterface> xess;
		static inline U8 xessRefCount = 0;
#endif

	public:
		InterfaceStorage() = delete;

#if _ZE_FFX_API_ENABLED
		static FfxApiInterface* CreateConnectionFfxApi(Device& dev) noexcept;
		static FfxApiInterface* GetConnectionFfxApi() noexcept;
		static void ReleaseConnectionFfxApi() noexcept;
#endif
#if _ZE_NGX_ENABLED
		static NgxInterface* CreateConnectionNGX(Device& dev) noexcept;
		static NgxInterface* GetConnectionNGX() noexcept;
		static void ReleaseConnectionNGX() noexcept;
#endif
#if _ZE_XESS_ENABLED
		static XeSSInterface* CreateConnectionXeSS(Device& dev) noexcept;
		static XeSSInterface* GetConnectionXeSS() noexcept;
		static void ReleaseConnectionXeSS() noexcept;
#endif
	};
}