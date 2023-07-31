#include "RHI/VK/SwapChain.h"
#include "RHI/VK/VulkanException.h"

namespace ZE::RHI::VK
{
	VkPresentModeKHR SwapChain::FindPresentMode(VkPhysicalDevice device)
	{
		ZE_VK_ENABLE();

		U32 count = 0;
		ZE_VK_THROW_NOSUCC(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr));
		ZE_ASSERT(count > 0, "Chosen surface should have enough present modes!");

		std::vector<VkPresentModeKHR> presentModes(count);
		ZE_VK_THROW_NOSUCC(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, presentModes.data()));

		// Check if tearing is supported
		if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != presentModes.end())
			return VK_PRESENT_MODE_IMMEDIATE_KHR;
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

		VkDeviceGroupSwapchainCreateInfoKHR deviceGroupInfo = { VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR, nullptr };
		deviceGroupInfo.modes = VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR;

		VkSwapchainCreateInfoKHR swapChainInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, &deviceGroupInfo };
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

		// Create fence to wait for new image to be available and semaphore for completion of rendering
		const VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0 };
		ZE_VK_THROW_NOSUCC(vkCreateFence(device, &fenceInfo, nullptr, &imageAquireFence));
		ZE_VK_SET_ID(device, imageAquireFence, VK_OBJECT_TYPE_FENCE, "swapchain_fence");

		const VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
		ZE_VK_THROW_NOSUCC(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &presentSemaphore));
		ZE_VK_SET_ID(device, presentSemaphore, VK_OBJECT_TYPE_SEMAPHORE, "present_semaphore");

		// Create image views from swapChain images
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		images = new VkImage[imageCount];
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images);

		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr };
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
			viewInfo.image = images[i];
			ZE_VK_THROW_NOSUCC(vkCreateImageView(device, &viewInfo, nullptr, views + i));
		}
	}

	SwapChain::~SwapChain()
	{
		ZE_ASSERT(views == nullptr && images == nullptr
			&& imageAquireFence == VK_NULL_HANDLE && presentSemaphore == VK_NULL_HANDLE
			&& swapChain == VK_NULL_HANDLE && surface == VK_NULL_HANDLE,
			"Free hasn't been called before destroying SwapChain!");
	}

	void SwapChain::StartFrame(GFX::Device& dev)
	{
		ZE_VK_ENABLE();

		// Get new swapChain image
		VkAcquireNextImageInfoKHR acquireInfo = { VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR, nullptr };
		acquireInfo.swapchain = swapChain;
		acquireInfo.timeout = UINT64_MAX;
		acquireInfo.semaphore = VK_NULL_HANDLE;
		acquireInfo.fence = imageAquireFence;
		acquireInfo.deviceMask = 1; // No device groups
		ZE_VK_THROW_NOSUCC(vkAcquireNextImage2KHR(dev.Get().vk.GetDevice(), &acquireInfo, &currentImage));
		ZE_VK_THROW_NOSUCC(vkWaitForFences(dev.Get().vk.GetDevice(), 1, &imageAquireFence, VK_TRUE, UINT64_MAX));
	}

	void SwapChain::Present(GFX::Device& dev) const
	{
		ZE_VK_ENABLE();

		// Present to new image
		const U32 mask = 1;
		VkDeviceGroupPresentInfoKHR deviceGroupInfo = { VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR, nullptr };
		deviceGroupInfo.swapchainCount = 1;
		deviceGroupInfo.pDeviceMasks = &mask;
		deviceGroupInfo.mode = VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR;

		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, &deviceGroupInfo };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &presentSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain;
		presentInfo.pImageIndices = &currentImage;
		presentInfo.pResults = nullptr;
		ZE_VK_THROW_NOSUCC(vkQueuePresentKHR(dev.Get().vk.GetGfxQueue(), &presentInfo));
	}

	void SwapChain::PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const
	{
		auto& device = dev.Get().vk;
		auto& list = cl.Get().vk;

		// Transition backbuffer to presentable layout
		VkImageMemoryBarrier2 presentBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, nullptr };
		presentBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | (device.CanPresentFromCompute() ? VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT : 0)
			| VK_PIPELINE_STAGE_2_COPY_BIT | VK_PIPELINE_STAGE_2_BLIT_BIT | VK_PIPELINE_STAGE_2_CLEAR_BIT;
		presentBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
		presentBarrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
		presentBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
		presentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // TODO: record last used access
		presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		presentBarrier.dstQueueFamilyIndex = presentBarrier.srcQueueFamilyIndex = list.GetFamily();
		presentBarrier.image = images[currentImage];
		presentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		VkDependencyInfo depInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO, nullptr };
		depInfo.dependencyFlags = 0;
		depInfo.memoryBarrierCount = 0;
		depInfo.pMemoryBarriers = nullptr;
		depInfo.bufferMemoryBarrierCount = 0;
		depInfo.pBufferMemoryBarriers = nullptr;
		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &presentBarrier;

		list.Open();
		vkCmdPipelineBarrier2(list.GetBuffer(), &depInfo);
		list.Close();
		ExecutePresentTransition(device, list);
	}

	void SwapChain::Free(GFX::Device& dev) noexcept
	{
		if (views)
		{
			for (U32 i = 0; i < imageCount; ++i)
				vkDestroyImageView(dev.Get().vk.GetDevice(), views[i], nullptr);
			views.DeleteArray();
		}
		if (images)
			views.DeleteArray();
		if (presentSemaphore)
		{
			vkDestroySemaphore(dev.Get().vk.GetDevice(), presentSemaphore, nullptr);
			presentSemaphore = VK_NULL_HANDLE;
		}
		if (imageAquireFence)
		{
			vkDestroyFence(dev.Get().vk.GetDevice(), imageAquireFence, nullptr);
			imageAquireFence = VK_NULL_HANDLE;
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

	void SwapChain::ExecutePresentTransition(Device& dev, CommandList& cl) const
	{
		ZE_VK_ENABLE();

		VkCommandBufferSubmitInfo bufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, nullptr };
		bufferInfo.commandBuffer = cl.GetBuffer();
		bufferInfo.deviceMask = 0;

		// Setup semaphore for present operation
		VkSemaphoreSubmitInfo signalInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr };
		signalInfo.semaphore = presentSemaphore;
		signalInfo.value = 0;
		signalInfo.stageMask = VK_PIPELINE_STAGE_2_NONE;
		signalInfo.deviceIndex = 0;

		VkSubmitInfo2 submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO_2, nullptr };
		submitInfo.flags = 0;
		submitInfo.waitSemaphoreInfoCount = 0;
		submitInfo.pWaitSemaphoreInfos = nullptr;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &bufferInfo;
		submitInfo.signalSemaphoreInfoCount = 1;
		submitInfo.pSignalSemaphoreInfos = &signalInfo;
		ZE_VK_THROW_NOSUCC(vkQueueSubmit2(dev.GetGfxQueue(), 1, &submitInfo, VK_NULL_HANDLE));
	}
}