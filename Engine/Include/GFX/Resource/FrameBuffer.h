#pragma once
#include "FrameBufferDesc.h"

namespace ZE::GFX::Resource
{
	class FrameBuffer final
	{
	public:
		FrameBuffer() = default;
		FrameBuffer(FrameBuffer&&) = delete;
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(FrameBuffer&&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;
		~FrameBuffer() = default;
	};
}