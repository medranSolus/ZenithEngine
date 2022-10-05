#pragma once
#include "GFX/Device.h"
#include "Window/MainWindow.h"

namespace ZE::GFX::API::VK
{
	class SwapChain final
	{
	public:
		SwapChain() = default;
		SwapChain(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput);
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain() = default;

		constexpr void PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const {}

		void Present(GFX::Device& dev) const;
	};
}