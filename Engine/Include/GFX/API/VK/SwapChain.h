#pragma once
#include "GFX/CommandList.h"

namespace ZE::GFX::API::VK
{
	class SwapChain final
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkSwapchainKHR swapChain = VK_NULL_HANDLE;
		VkFence imageAquireFence = VK_NULL_HANDLE;
		VkSemaphore presentSemaphore = VK_NULL_HANDLE;
		// Should be number of backbuffers but some implementations create more than asked for
		U32 imageCount = 0;
		U32 currentImage = 0;
		Ptr<VkImage> images;
		Ptr<VkImageView> views;

		VkPresentModeKHR FindPresentMode(VkPhysicalDevice device);
		VkSurfaceFormat2KHR FindSurfaceFormat(VkPhysicalDevice device);

	public:
		SwapChain() = default;
		SwapChain(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput);
		ZE_CLASS_MOVE(SwapChain);
		~SwapChain();

		void StartFrame(GFX::Device& dev);
		void Present(GFX::Device& dev) const;
		void PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const;
		void Free(GFX::Device& dev) noexcept;

		// Gfx API Internal

		constexpr VkImageView GetCurrentView() const noexcept { return views[currentImage]; }

		void ExecutePresentTransition(Device& dev, CommandList& cl) const;
	};
}