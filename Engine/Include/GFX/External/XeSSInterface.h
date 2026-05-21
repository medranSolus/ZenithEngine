#pragma once
#if _ZE_XESS_ENABLED
#	if _ZE_RHI_DX11
#		include "RHI/DX11/External/XeSSInterface.h"
#	endif
#	if _ZE_RHI_DX12
#		include "RHI/DX12/External/XeSSInterface.h"
#	endif
#	if _ZE_RHI_VK
#		include "RHI/VK/External/XeSSInterface.h"
#	endif
#	include "RHI/Backend.h"

namespace ZE::GFX::External
{
	// Helper class for accessing XeSS methods
	class XeSSInterface final
	{
		ZE_RHI_BACKEND(External::XeSSInterface);

	public:
		XeSSInterface() = default;
		ZE_CLASS_MOVE(External::XeSSInterface);
		~XeSSInterface() = default;

		static Expected<XeSSInterface> Create(Device& dev) noexcept { ZE_RHI_BACKEND_CREATE(External::XeSSInterface, dev); }
		ZE_RHI_BACKEND_GET(External::XeSSInterface);
		// Main Gfx Api

		constexpr bool IsInitialized() const noexcept { ZE_RHI_BACKEND_CALL_RET(IsInitialized); }
		constexpr bool IsCtxInitialized() const noexcept { ZE_RHI_BACKEND_CALL_RET(IsCtxInitialized); }
		constexpr xess_context_handle_t GetCtx() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetCtx); }
		constexpr bool IsAliasableResourcesSupported() const noexcept { ZE_RHI_BACKEND_CALL_RET(IsAliasableResourcesSupported); }
		constexpr U64 GetAliasableBufferRegionSize() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetAliasableBufferRegionSize); }
		constexpr U64 GetAliasableTextureRegionSize() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetAliasableTextureRegionSize); }
		constexpr void SetAliasableResources(RID buffer, RID texture) noexcept { ZE_RHI_BACKEND_CALL(SetAliasableResources, buffer, texture); }

		Status InitializeCtx(Device& dev, UInt2 targetRes, xess_quality_settings_t quality, U32 flags) noexcept { ZE_RHI_BACKEND_CALL_RET(InitializeCtx, dev, targetRes, quality, flags); }
		// May invalidate ctx handle
		Status FreeCtx(Device& dev) noexcept { ZE_RHI_BACKEND_CALL_RET(FreeCtx, dev); }

		// Depth, exposure and responsive parameters are optional, when this buffers are not present then pass in INVALID_RID
		Status Execute(GFX::Device& dev, GFX::Pipeline::FrameBuffer& buffers, GFX::CommandList& cl, RID color, RID motionVectors, RID depth, RID exposure, RID responsive, RID output, const Float2& jitter, bool reset) const noexcept { ZE_RHI_BACKEND_CALL_RET(Execute, dev, buffers, cl, color, motionVectors, depth, exposure, responsive, output, jitter, reset); }
	};
}
#endif