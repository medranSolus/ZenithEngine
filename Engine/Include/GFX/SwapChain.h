#pragma once
#include "Device.h"

namespace ZE::GFX
{
	// Managing backbuffers
	class SwapChain
	{
	protected:
		SwapChain() = default;

	public:
		virtual ~SwapChain() = default;

		virtual void Present(Device& dev) const = 0;
	};
}