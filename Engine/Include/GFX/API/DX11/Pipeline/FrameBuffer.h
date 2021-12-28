#pragma once
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/SwapChain.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Pipeline
{
	class FrameBuffer final
	{
	public:
		FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList,
			const GFX::Pipeline::FrameBufferDesc& desc);
		ZE_CLASS_DELETE(FrameBuffer);
		~FrameBuffer() = default;

		void InitRTV(GFX::CommandList& cl, RID rid) const noexcept {}
		void InitDSV(GFX::CommandList& cl, RID rid) const noexcept {}

		void SetRTV(GFX::Device& dev, GFX::CommandList& cl, RID rid) const {}
		void SetDSV(GFX::Device& dev, GFX::CommandList& cl, RID rid) const {}
		void SetOutput(GFX::Device& dev, GFX::CommandList& cl, RID rtv, RID dsv) const {}

		template<U32 RTVCount>
		void SetRTV(GFX::Device& dev, GFX::CommandList& cl, const RID* rid) const {}
		template<U32 RTVCount>
		void SetOutput(GFX::Device& dev, GFX::CommandList& cl, const RID* rtv, RID dsv) const {}

		void ClearRTV(GFX::Device& dev, GFX::CommandList& cl, RID rid, const ColorF4 color) const {}
		void ClearDSV(GFX::Device& dev, GFX::CommandList& cl, RID rid, float depth, U8 stencil) const {}

		constexpr void SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) {}
		constexpr void InitTransitions(GFX::Device& dev, GFX::CommandList& cl) const {}
		constexpr void ExitTransitions(GFX::Device& dev, GFX::CommandList& cl, U64 level) const noexcept {}
	};
}