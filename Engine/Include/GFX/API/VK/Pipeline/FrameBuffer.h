#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/SwapChain.h"

namespace ZE::GFX::API::VK::Pipeline
{
	class FrameBuffer final
	{
	public:
		FrameBuffer() = default;
		FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList,
			const GFX::Pipeline::FrameBufferDesc& desc);
		ZE_CLASS_DELETE(FrameBuffer);
		~FrameBuffer();

		constexpr UInt2 GetDimmensions(RID rid) const noexcept { return { 0, 0 }; }

		constexpr void InitRTV(GFX::CommandList& cl, RID rid) const noexcept {}
		constexpr void InitDSV(GFX::CommandList& cl, RID rid) const noexcept {}

		void Copy(GFX::CommandList& cl, RID src, RID dest) const noexcept;

		void SetRTV(GFX::CommandList& cl, RID rid) const noexcept;
		void SetRTV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept;
		void SetDSV(GFX::CommandList& cl, RID rid) const noexcept;
		void SetDSV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept;
		void SetOutput(GFX::CommandList& cl, RID rtv, RID dsv) const noexcept;

		template<U32 RTVCount>
		void SetRTV(GFX::CommandList& cl, const RID* rid, bool adjacent) const noexcept;
		template<U32 RTVCount>
		void SetOutput(GFX::CommandList& cl, const RID* rtv, RID dsv, bool adjacent) const noexcept;

		void SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept;
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept;
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid, U16 mipLevel) const noexcept;

		constexpr void BarrierUAV(GFX::CommandList& cl, RID rid) const noexcept {}
		void BarrierTransition(GFX::CommandList& cl, RID rid, GFX::Resource::State before, GFX::Resource::State after) const noexcept;
		template<U32 BarrierCount>
		void BarrierTransition(GFX::CommandList& cl, const std::array<GFX::Pipeline::TransitionInfo, BarrierCount>& barriers) const noexcept;

		void ClearRTV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept;
		void ClearDSV(GFX::CommandList& cl, RID rid, float depth, U8 stencil) const noexcept;
		void ClearUAV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept;
		void ClearUAV(GFX::CommandList& cl, RID rid, const Pixel colors[4]) const noexcept;

		void SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept;
		constexpr void InitTransitions(GFX::Device& dev, GFX::CommandList& cl) const noexcept {}
		void ExitTransitions(GFX::Device& dev, GFX::CommandList& cl, U64 level) const noexcept;
	};

#pragma region Functions
	template<U32 RTVCount>
	void FrameBuffer::SetRTV(GFX::CommandList& cl, const RID* rid, bool adjacent) const noexcept
	{
	}

	template<U32 RTVCount>
	void FrameBuffer::SetOutput(GFX::CommandList& cl, const RID* rtv, RID dsv, bool adjacent) const noexcept
	{
	}

	template<U32 BarrierCount>
	void FrameBuffer::BarrierTransition(GFX::CommandList& cl, const std::array<GFX::Pipeline::TransitionInfo, BarrierCount>& barriers) const noexcept
	{
	}
#pragma endregion
}