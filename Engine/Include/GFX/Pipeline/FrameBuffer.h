#pragma once
#include "FrameBufferDesc.h"
#include "TransitionDesc.h"

namespace ZE::GFX::Pipeline
{
	// Managing all writeable buffers used during single frame
	class FrameBuffer final
	{
		static std::vector<std::vector<TransitionDesc>> GetTransitionsPerLevel(FrameBufferDesc& desc) noexcept;

	public:
		FrameBuffer() = default;
		FrameBuffer(FrameBuffer&&) = delete;
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(FrameBuffer&&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;
		~FrameBuffer() = default;

		void Init(FrameBufferDesc& desc) { GetTransitionsPerLevel(desc); }
	};
}