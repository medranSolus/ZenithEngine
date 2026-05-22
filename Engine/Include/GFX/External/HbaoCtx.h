#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/External/HbaoCtx.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/External/HbaoCtx.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/External/HbaoCtx.h"
#endif

namespace ZE::GFX::External
{
	// Helper class for accessing XeSS methods
	class HbaoCtx final
	{
		ZE_RHI_BACKEND(External::HbaoCtx);

	public:
		HbaoCtx() = default;
		ZE_CLASS_MOVE(External::HbaoCtx);
		~HbaoCtx() = default;

		static Expected<HbaoCtx> Create(Device& dev) noexcept { ZE_RHI_BACKEND_CREATE(External::HbaoCtx, dev); }
		ZE_RHI_BACKEND_GET(External::HbaoCtx);

		// Main Gfx Api

		// Can be called optionally to trigger resource creation early and avoid stall during render call
		Status CreateResources(Device& dev, const GFSDK_SSAO_Parameters& params, UInt2 renderSize) noexcept { ZE_RHI_BACKEND_CALL_RET(CreateResources, dev, params, renderSize); }
		Status Render(Device& dev, Pipeline::FrameBuffer& buffers, const GFSDK_SSAO_Parameters& params, RID depth, RID normals, RID output, bool blendMultiply = false, bool linearDepth = false) noexcept { ZE_RHI_BACKEND_CALL_RET(Render, dev, buffers, params, depth, normals, output, blendMultiply, linearDepth); }
	};
}