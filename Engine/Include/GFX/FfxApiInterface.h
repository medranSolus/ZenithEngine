#pragma once
#if _ZE_FFX_API_ENABLED
#	if _ZE_RHI_DX11
#		include "RHI/DX11/FfxApiInterface.h"
#	endif
#	if _ZE_RHI_DX12
#		include "RHI/DX12/FfxApiInterface.h"
#	endif
#	if _ZE_RHI_VK
#		include "RHI/VK/FfxApiInterface.h"
#	endif
#	include "RHI/Backend.h"

namespace ZE::GFX
{
	// Helper class for accessing FFX API methods
	class FfxApiInterface final
	{
		ZE_RHI_BACKEND(FfxApiInterface);

	public:
		FfxApiInterface() = default;
		ZE_CLASS_MOVE(FfxApiInterface);
		~FfxApiInterface() = default;

		static Expected<FfxApiInterface> Create() noexcept { ZE_RHI_BACKEND_CREATE(FfxApiInterface); }
		ZE_RHI_BACKEND_GET(FfxApiInterface);

		// Main Gfx Api

		constexpr bool IsInitialized() const noexcept {  ZE_RHI_BACKEND_CALL_RET(IsInitialized); }
		constexpr const FfxApiFunctions& GetFunctions() const noexcept { const FfxApiFunctions* func = nullptr; ZE_RHI_BACKEND_CALL_RET_VAR(func, GetFunctions); return *func; }
		constexpr ffxReturnCode_t CreateFfxCtx(Device& dev, ffxContext* ctx, ffxCreateContextDescHeader& ctxHeader) const noexcept { ZE_RHI_BACKEND_CALL_RET(CreateFfxCtx, dev, ctx, ctxHeader); }
	};
}
#endif