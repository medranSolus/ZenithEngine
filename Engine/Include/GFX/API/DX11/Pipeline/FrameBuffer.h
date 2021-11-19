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
		FrameBuffer(FrameBuffer&&) = default;
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(FrameBuffer&&) = default;
		FrameBuffer& operator=(const FrameBuffer&) = delete;
		~FrameBuffer() = default;

		void InitRTV(GFX::CommandList& cl, U64 rid) const noexcept {}
		void InitDSV(GFX::CommandList& cl, U64 rid) const noexcept {}

		void ClearRTV(GFX::Device& dev, GFX::CommandList& cl, U64 rid, const ColorF4 color) const {}
		void ClearDSV(GFX::Device& dev, GFX::CommandList& cl, U64 rid, float depth, U8 stencil) const {}

		constexpr void SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) {}
		constexpr void InitTransitions(GFX::Device& dev, GFX::CommandList& cl) const {}
		constexpr void ExitTransitions(U64 level, GFX::CommandList& cl) const noexcept {}
	};
}