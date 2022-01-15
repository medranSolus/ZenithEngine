#pragma once
#include "GFX/Binding/Context.h"
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

		void SetRTV(GFX::CommandList& cl, RID rid) const {}
		void SetDSV(GFX::CommandList& cl, RID rid) const {}
		void SetOutput(GFX::CommandList& cl, RID rtv, RID dsv) const {}

		template<U32 RTVCount>
		void SetRTV(GFX::CommandList& cl, const RID* rid) const {}
		template<U32 RTVCount>
		void SetOutput(GFX::CommandList& cl, const RID* rtv, RID dsv) const {}

		void SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const {}
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const {}

		void ClearRTV(GFX::CommandList& cl, RID rid, const ColorF4& color) const {}
		void ClearDSV(GFX::CommandList& cl, RID rid, float depth, U8 stencil) const {}
		void ClearUAV(GFX::CommandList& cl, RID rid, const ColorF4& color) const {}
		void ClearUAV(GFX::CommandList& cl, RID rid, const Pixel colors[4]) const {}

		constexpr void SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) {}
		constexpr void InitTransitions(GFX::Device& dev, GFX::CommandList& cl) const {}
		constexpr void ExitTransitions(GFX::Device& dev, GFX::CommandList& cl, U64 level) const noexcept {}
	};
}