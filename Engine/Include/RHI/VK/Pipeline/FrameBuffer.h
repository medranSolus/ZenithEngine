#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/SwapChain.h"

namespace ZE::RHI::VK::Pipeline
{
	class FrameBuffer final
	{
	public:
		FrameBuffer() = default;
		FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList,
			const GFX::Pipeline::FrameBufferDesc& desc);
		ZE_CLASS_DELETE(FrameBuffer);
		~FrameBuffer();

		void BeginRaster(GFX::CommandList& cl);
		void EndRaster(GFX::CommandList& cl);
	};

#pragma region Functions
#pragma endregion
}