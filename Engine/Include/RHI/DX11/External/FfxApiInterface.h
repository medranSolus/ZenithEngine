#pragma once
#if _ZE_FFX_API_ENABLED
#	include "GFX/External/FfxApiFunctions.h"
#	include "GFX/External/Error.h"
#	include "GFX/Device.h"

namespace ZE::RHI::DX11::External
{
	class FfxApiInterface final
	{
	public:
		FfxApiInterface() = default;
		ZE_CLASS_MOVE(FfxApiInterface);
		~FfxApiInterface() = default;

		static Expected<FfxApiInterface> Create() noexcept { return std::unexpected(ZE_FFX_API_ERROR(FFX_API_RETURN_ERROR)); }

		constexpr bool IsInitialized() const noexcept { return false; }
		constexpr const GFX::External::FfxApiFunctions* GetFunctions() const noexcept { return nullptr; }

		ffxReturnCode_t CreateFfxCtx(GFX::Device& dev, ffxContext* ctx, ffxCreateContextDescHeader& ctxHeader) const noexcept { return FFX_API_RETURN_ERROR; }
	};
}
#endif