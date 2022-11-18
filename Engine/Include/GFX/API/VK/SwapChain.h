#pragma once
#include "GFX/Device.h"

namespace ZE::GFX::API::VK
{
	class SwapChain final
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkSwapchainKHR swapChain = VK_NULL_HANDLE;
		// Should be number of backbuffers but some implementations create more than asked for
		U32 imageCount = 0;
		Ptr<VkImageView> views;

		VkPresentModeKHR FindPresentMode(VkPhysicalDevice device);
		VkSurfaceFormat2KHR FindSurfaceFormat(VkPhysicalDevice device);

	public:
		SwapChain() = default;
		SwapChain(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput);
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain();

		void Present(GFX::Device& dev) const;
		void PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const;
		void Free(GFX::Device& dev) noexcept;
	};
}