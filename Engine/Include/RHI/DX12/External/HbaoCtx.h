#pragma once
#include "GFX/Pipeline/FrameBuffer.h"
ZE_WARNING_PUSH
#include "GFSDK_SSAO.h"
ZE_WARNING_POP

namespace ZE::RHI::DX12::External
{
	class HbaoCtx final
	{
		Ptr<GFSDK_SSAO_Context_D3D12> ctx;
		DX::ComPtr<IDescriptorHeap> rtvDescHeap;
		DescriptorInfo srvDescInfo = {};

	public:
		HbaoCtx() = default;
		ZE_CLASS_MOVE(HbaoCtx);
		~HbaoCtx();

		static Expected<HbaoCtx> Create(GFX::Device& dev) noexcept;

		Status CreateResources(GFX::Device& dev, const GFSDK_SSAO_Parameters& params, UInt2 renderSize) noexcept;
		Status Render(GFX::Device& dev, GFX::Pipeline::FrameBuffer& buffers, const GFSDK_SSAO_Parameters& params,
			RID depth, RID normals, RID output, bool blendMultiply = false, bool linearDepth = false) noexcept;
	};
}