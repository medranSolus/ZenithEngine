#include "GFX/API/VK/Pipeline/FrameBuffer.h"

namespace ZE::GFX::API::VK::Pipeline
{
	FrameBuffer::FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList,
		const GFX::Pipeline::FrameBufferDesc& desc)
	{
	}

	FrameBuffer::~FrameBuffer()
	{
	}

	void FrameBuffer::Copy(GFX::CommandList& cl, RID src, RID dest) const noexcept
	{
	}

	void FrameBuffer::SetRTV(GFX::CommandList& cl, RID rid) const noexcept
	{
	}

	void FrameBuffer::SetRTV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept
	{
	}

	void FrameBuffer::SetDSV(GFX::CommandList& cl, RID rid) const noexcept
	{
	}

	void FrameBuffer::SetDSV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept
	{
	}

	void FrameBuffer::SetOutput(GFX::CommandList& cl, RID rtv, RID dsv) const noexcept
	{
	}

	void FrameBuffer::SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept
	{
	}

	void FrameBuffer::SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept
	{
	}

	void FrameBuffer::SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid, U16 mipLevel) const noexcept
	{
	}

	void FrameBuffer::BarrierTransition(GFX::CommandList& cl, RID rid, GFX::Resource::State before, GFX::Resource::State after) const noexcept
	{
	}

	void FrameBuffer::ClearRTV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept
	{
	}

	void FrameBuffer::ClearDSV(GFX::CommandList& cl, RID rid, float depth, U8 stencil) const noexcept
	{
	}

	void FrameBuffer::ClearUAV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept
	{
	}

	void FrameBuffer::ClearUAV(GFX::CommandList& cl, RID rid, const Pixel colors[4]) const noexcept
	{
	}

	void FrameBuffer::SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept
	{
	}

	void FrameBuffer::ExitTransitions(GFX::Device& dev, GFX::CommandList& cl, U64 level) const noexcept
	{
	}
}