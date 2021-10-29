#pragma once
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/SwapChain.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Pipeline
{
	class FrameBuffer final
	{
	public:
		FrameBuffer(GFX::Device& dev, GFX::SwapChain& swapChain,
			const GFX::Pipeline::FrameBufferDesc& desc);
		FrameBuffer(FrameBuffer&&) = default;
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(FrameBuffer&&) = default;
		FrameBuffer& operator=(const FrameBuffer&) = delete;
		~FrameBuffer() = default;
	};
}