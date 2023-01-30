#include "GFX/API/VK/SwapChain.h"
#include "GFX/API/VK/VulkanException.h"

namespace ZE::GFX::API::VK
{
	VkPresentModeKHR SwapChain::FindPresentMode(VkPhysicalDevice device)
	{
		ZE_VK_ENABLE();

		U32 count = 0;
		ZE_VK_THROW_NOSUCC(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr));
		ZE_ASSERT(count > 0, "Chosen surface should have enough present modes!");

		std::vector<VkPresentModeKHR> presentModes(count);
		ZE_VK_THROW_NOSUCC(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, presentModes.data()));
		for (VkPresentModeKHR mode : presentModes)
		{
			// Check if tearing is supported
			if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				return VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
		// FIFO always guaranteed to be present
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkSurfaceFormat2KHR SwapChain::FindSurfaceFormat(VkPhysicalDevice device)
	{
		ZE_VK_ENABLE();

		U32 count = 0;
		const VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, nullptr, surface };
		ZE_VK_THROW_NOSUCC(vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surfaceInfo, &count, nullptr));
		ZE_ASSERT(count > 0, "Chosen surface should have enough supported formats!");

		std::vector<VkSurfaceFormat2KHR> supportedFormats(count, { VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR, nullptr });
		ZE_VK_THROW_NOSUCC(vkGetPhysicalDeviceSurfaceFormats2KHR(device, &surfaceInfo, &count, supportedFormats.data()));
		do
		{
			const PixelFormat format = GetFormatFromVk(supportedFormats.at(--count).surfaceFormat.format);
			if (!Utils::IsSameFormatFamily(format, Settings::GetBackbufferFormat()))
				supportedFormats.erase(supportedFormats.begin() + count);
		} while (count > 0);
		ZE_ASSERT(supportedFormats.size() > 0, "For selected surface there should be at least one suitable format!");

		// Skip all the hassle with selecting proper format
		if (supportedFormats.size() == 1)
			return supportedFormats.front();

		// When more than one format then select first one that matches the backbuffer
		for (const VkSurfaceFormat2KHR& format : supportedFormats)
		{
			if (GetFormatFromVk(format.surfaceFormat.format) == Settings::GetBackbufferFormat())
				return format;
		}
		// Otherwise just go with first one
		return supportedFormats.front();
	}

	SwapChain::SwapChain(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput)
	{
		ZE_VK_ENABLE_ID();
		VkPhysicalDevice phyDevice = dev.Get().vk.GetPhysicalDevice();
		VkDevice device = dev.Get().vk.GetDevice();

		surface = CreateSurface(window, dev.Get().vk.GetInstance());
		ZE_VK_SET_ID(dev.Get().vk.GetDevice(), surface, VK_OBJECT_TYPE_SURFACE_KHR, "window_surface");

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		ZE_VK_THROW_NOSUCC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDevice, surface, &surfaceCapabilities));
		// This should never happen if sticking to the typical values like 2 or 3
		if (surfaceCapabilities.minImageCount > Settings::GetBackbufferCount()
			|| (surfaceCapabilities.maxImageCount < Settings::GetBackbufferCount() && surfaceCapabilities.maxImageCount > 0))
		{
			throw ZE_CMP_EXCEPT("Requested backbuffer count is not supported by current Vulkan implementation [" +
				std::to_string(surfaceCapabilities.minImageCount) + ";" + std::to_string(surfaceCapabilities.maxImageCount) +
				"]! Requested:" + std::to_string(Settings::GetBackbufferCount()));
		}

		// Find surface format and select it if different from desired
		// Ignore color space for now until HDR is used
		const VkSurfaceFormat2KHR surfaceFormat = FindSurfaceFormat(phyDevice);
		const PixelFormat swapChainFormat = GetFormatFromVk(surfaceFormat.surfaceFormat.format);
		if (swapChainFormat != Settings::GetBackbufferFormat())
			Settings::SetBackbufferFormat(swapChainFormat);

		VkSwapchainCreateInfoKHR swapChainInfo;
		swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainInfo.pNext = nullptr;
		swapChainInfo.flags = 0;
		swapChainInfo.surface = surface;
		swapChainInfo.minImageCount = Settings::GetBackbufferCount();
		swapChainInfo.imageFormat = surfaceFormat.surfaceFormat.format;
		swapChainInfo.imageColorSpace = surfaceFormat.surfaceFormat.colorSpace;
		// Driver allows to choose extent ourselves
		if (surfaceCapabilities.currentExtent.width == UINT32_MAX)
		{
			swapChainInfo.imageExtent.width = std::clamp(window.GetWidth(), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			swapChainInfo.imageExtent.height = std::clamp(window.GetHeight(), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		}
		else
			swapChainInfo.imageExtent = surfaceCapabilities.currentExtent;
		swapChainInfo.imageArrayLayers = 1; // Don't care about 3D images (stereoscopic usage)
		// DST in case of copy target, SRC in case of saving screenshot, COLOR for rendering, INPUT and SAMPLED for using as shader resource
		swapChainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			| (shaderInput ? VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT : 0);
		swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainInfo.queueFamilyIndexCount = 0;
		swapChainInfo.pQueueFamilyIndices = nullptr;
		swapChainInfo.preTransform = surfaceCapabilities.currentTransform;
		swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainInfo.presentMode = FindPresentMode(phyDevice);
		swapChainInfo.clipped = VK_TRUE;
		swapChainInfo.oldSwapchain = VK_NULL_HANDLE;
		ZE_VK_THROW_NOSUCC(vkCreateSwapchainKHR(device, &swapChainInfo, nullptr, &swapChain));
		ZE_VK_SET_ID(device, swapChain, VK_OBJECT_TYPE_SWAPCHAIN_KHR, "main_swapchain");

		// Create image views from swapChain images
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		std::vector<VkImage> images(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());

		VkImageViewCreateInfo viewInfo;
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = swapChainInfo.imageFormat;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		views = new VkImageView[imageCount];
		for (U32 i = 0; i < imageCount; ++i)
		{
			viewInfo.image = images.at(i);
			ZE_VK_THROW_NOSUCC(vkCreateImageView(device, &viewInfo, nullptr, views + i));
		}
	}

	SwapChain::~SwapChain()
	{
		ZE_ASSERT(views == nullptr && swapChain == VK_NULL_HANDLE && surface == VK_NULL_HANDLE,
			"Free hasn't been called before destroying SwapChain!");
	}

	void SwapChain::Present(GFX::Device& dev) const
	{
		ZE_VK_ENABLE();

		// Get new swapChain image
		U32 imageIndex = Settings::GetFrameIndex() % imageCount;
		VkAcquireNextImageInfoKHR acquireInfo;
		acquireInfo.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
		acquireInfo.pNext = nullptr;
		acquireInfo.swapchain = swapChain;
		acquireInfo.timeout = UINT64_MAX;
		acquireInfo.semaphore = VK_NULL_HANDLE;
		acquireInfo.fence = VK_NULL_HANDLE;
		acquireInfo.deviceMask = 1; // No device groups
		ZE_VK_THROW_NOSUCC(vkAcquireNextImage2KHR(dev.Get().vk.GetDevice(), &acquireInfo, &imageIndex));

		// Present to new image
		VkPresentInfoKHR presentInfo;
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 0;
		presentInfo.pWaitSemaphores = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;
		ZE_VK_THROW_NOSUCC(vkQueuePresentKHR(dev.Get().vk.GetGfxQueue(), &presentInfo));
	}

	void SwapChain::PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const
	{
		// TODO: need to transition to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	}

	void SwapChain::Free(GFX::Device& dev) noexcept
	{
		if (views)
		{
			for (U32 i = 0; i < imageCount; ++i)
				vkDestroyImageView(dev.Get().vk.GetDevice(), views[i], nullptr);
			views.DeleteArray();
		}
		if (swapChain)
		{
			vkDestroySwapchainKHR(dev.Get().vk.GetDevice(), swapChain, nullptr);
			swapChain = VK_NULL_HANDLE;
		}
		if (surface)
		{
			vkDestroySurfaceKHR(dev.Get().vk.GetInstance(), surface, nullptr);
			surface = VK_NULL_HANDLE;
		}
	}
}