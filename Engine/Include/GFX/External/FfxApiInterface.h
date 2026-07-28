#pragma once
#if _ZE_FFX_API_ENABLED
#	if _ZE_RHI_DX11
#		include "RHI/DX11/External/FfxApiInterface.h"
#	endif
#	if _ZE_RHI_DX12
#		include "RHI/DX12/External/FfxApiInterface.h"
#	endif
#	if _ZE_RHI_VK
#		include "RHI/VK/External/FfxApiInterface.h"
#	endif
#	include "RHI/Backend.h"

namespace ZE::GFX::External
{
	// Helper class for accessing FFX API methods
	class FfxApiInterface final
	{
		ZE_RHI_BACKEND(External::FfxApiInterface);

	public:
		FfxApiInterface() = default;
		ZE_CLASS_MOVE(External::FfxApiInterface);
		~FfxApiInterface() = default;

		static Expected<FfxApiInterface> Create(Device& dev) noexcept { ZE_RHI_BACKEND_CREATE(External::FfxApiInterface, dev); }
		ZE_RHI_BACKEND_GET(External::FfxApiInterface);

		// Main Gfx Api

		constexpr bool IsInitialized() const noexcept {  ZE_RHI_BACKEND_CALL_RET(IsInitialized); }
		constexpr const FfxApiFunctions& GetFunctions() const noexcept { const FfxApiFunctions* func = nullptr; ZE_RHI_BACKEND_CALL_RET_VAR(func, GetFunctions); return *func; }
		constexpr ffxReturnCode_t CreateFfxCtx(Device& dev, Pipeline::FrameBuffer& buffers, ffxContext* ctx, ffxCreateContextDescHeader& ctxHeader, RID aliasableRegion) noexcept { ZE_RHI_BACKEND_CALL_RET(CreateFfxCtx, dev, buffers, ctx, ctxHeader, aliasableRegion); }
	};
}
#endif