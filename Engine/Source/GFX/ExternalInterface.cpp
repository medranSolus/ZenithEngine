#include "GFX/ExternalInterface.h"

namespace ZE::GFX
{
#if _ZE_FFX_API_ENABLED
	FfxApiInterface* ExternalInterface::CreateConnectionFfxApi() noexcept
	{
		if (ffxRefCount == 0)
		{
			auto exp = FfxApiInterface::Create();
			if (!exp)
			{
				ZE_CODE_ERROR(exp.error(), "Failed to initialize FFX API interface!");
				return nullptr;
			}
			ffx = std::move(*exp);
		}
		++ffxRefCount;
		return &ffx;
	}

	FfxApiInterface* ExternalInterface::GetConnectionFfxApi() noexcept
	{
		if (ffxRefCount)
		{
			ZE_ASSERT(ffx.IsInitialized(), "Ref count incorrect, FFX API interface not initialized!");
			return &ffx;
		}
		return nullptr;
	}

	void ExternalInterface::ReleaseConnectionFfxApi() noexcept
	{
		if (ffxRefCount)
		{
			if (--ffxRefCount == 0)
				ffx = {};
		}
	}
#endif

#if _ZE_NGX_ENABLED
	NgxInterface* ExternalInterface::CreateConnectionNGX(Device& dev) noexcept
	{
		if (Settings::GpuVendor != VendorGPU::Nvidia)
			return nullptr;
		if (ngxRefCount == 0)
		{
			auto exp = NgxInterface::Create(dev);
			if (!exp)
			{
				ZE_CODE_ERROR(exp.error(), "Failed to initialize NGX interface!");
				return nullptr;
			}
			ngx = std::move(*exp);
		}
		++ngxRefCount;
		return &ngx;
	}

	NgxInterface* ExternalInterface::GetConnectionNGX() noexcept
	{
		if (ngxRefCount)
		{
			ZE_ASSERT(ngx.IsInitialized(), "Ref count incorrect, NGX interface not initialized!");
			return &ngx;
		}
		return nullptr;
	}

	void ExternalInterface::ReleaseConnectionNGX() noexcept
	{
		if (ngxRefCount)
		{
			if (--ngxRefCount == 0)
				ngx = {};
		}
	}
#endif

#if _ZE_XESS_ENABLED
	XeSSInterface* ExternalInterface::CreateConnectionXeSS(Device& dev) noexcept
	{
		if (xessRefCount == 0)
		{
			auto exp = XeSSInterface::Create(dev);
			if (!exp)
			{
				ZE_CODE_ERROR(exp.error(), "Failed to initialize XeSS interface!");
				return nullptr;
			}
			xess = std::move(*exp);
		}
		++xessRefCount;
		return &xess;
	}

	XeSSInterface* ExternalInterface::GetConnectionXeSS() noexcept
	{
		if (xessRefCount)
		{
			ZE_ASSERT(xess.IsInitialized(), "Ref count incorrect, XeSS interface not initialized!");
			return &xess;
		}
		return nullptr;
	}

	void ExternalInterface::ReleaseConnectionXeSS() noexcept
	{
		if (xessRefCount)
		{
			if (--xessRefCount == 0)
				xess = {};
		}
	}
#endif
}