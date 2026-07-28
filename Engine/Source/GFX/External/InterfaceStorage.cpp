#include "GFX/External/InterfaceStorage.h"

namespace ZE::GFX::External
{
#if _ZE_FFX_API_ENABLED
	FfxApiInterface* InterfaceStorage::CreateConnectionFfxApi() noexcept
	{
		if (ffxRefCount == 0)
		{
			auto exp = FfxApiInterface::Create();
			if (!exp)
			{
				ZE_CODE_ERROR(exp.error(), "Failed to initialize FFX API interface!");
				return nullptr;
			}
			ffx = std::make_unique<FfxApiInterface>(std::move(*exp));
		}
		++ffxRefCount;
		return ffx.get();
	}

	FfxApiInterface* InterfaceStorage::GetConnectionFfxApi() noexcept
	{
		if (ffxRefCount)
		{
			ZE_ASSERT(ffx && ffx->IsInitialized(), "Ref count incorrect, FFX API interface not initialized!");
			return ffx.get();
		}
		return nullptr;
	}

	void InterfaceStorage::ReleaseConnectionFfxApi() noexcept
	{
		if (ffxRefCount)
		{
			if (--ffxRefCount == 0)
				ffx = nullptr;
		}
	}
#endif

#if _ZE_NGX_ENABLED
	NgxInterface* InterfaceStorage::CreateConnectionNGX(Device& dev) noexcept
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
			ngx = std::make_unique<NgxInterface>(std::move(*exp));
		}
		++ngxRefCount;
		return ngx.get();
	}

	NgxInterface* InterfaceStorage::GetConnectionNGX() noexcept
	{
		if (ngxRefCount)
		{
			ZE_ASSERT(ngx && ngx->IsInitialized(), "Ref count incorrect, NGX interface not initialized!");
			return ngx.get();
		}
		return nullptr;
	}

	void InterfaceStorage::ReleaseConnectionNGX() noexcept
	{
		if (ngxRefCount)
		{
			if (--ngxRefCount == 0)
				ngx = nullptr;
		}
	}
#endif

#if _ZE_XESS_ENABLED
	XeSSInterface* InterfaceStorage::CreateConnectionXeSS(Device& dev) noexcept
	{
		if (xessRefCount == 0)
		{
			auto exp = XeSSInterface::Create(dev);
			if (!exp)
			{
				ZE_CODE_ERROR(exp.error(), "Failed to initialize XeSS interface!");
				return nullptr;
			}
			xess = std::make_unique<XeSSInterface>(std::move(*exp));
		}
		++xessRefCount;
		return xess.get();
	}

	XeSSInterface* InterfaceStorage::GetConnectionXeSS() noexcept
	{
		if (xessRefCount)
		{
			ZE_ASSERT(xess && xess->IsInitialized(), "Ref count incorrect, XeSS interface not initialized!");
			return xess.get();
		}
		return nullptr;
	}

	void InterfaceStorage::ReleaseConnectionXeSS() noexcept
	{
		if (xessRefCount)
		{
			if (--xessRefCount == 0)
				xess = nullptr;
		}
	}
#endif
}