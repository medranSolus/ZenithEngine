#pragma once
#include "GFX/Device.h"

namespace ZE::GFX::API::VK
{
	class SwapChain final
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;

	public:
		SwapChain() = default;
		SwapChain(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput);
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain();

		constexpr void PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const {}

		void Present(GFX::Device& dev) const;
	};
}