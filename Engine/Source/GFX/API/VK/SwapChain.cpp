#include "GFX/API/VK/SwapChain.h"
#include "GFX/API/VK/VulkanException.h"

namespace ZE::GFX::API::VK
{
	SwapChain::SwapChain(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput)
	{
		ZE_VK_ENABLE();

		surface = CreateSurface(window, dev.Get().vk.GetInstance());
	}

	SwapChain::~SwapChain()
	{
		if (surface)
			vkDestroySurfaceKHR(static_cast<VkInstance>(Settings::GetGfxCustomData()), surface, nullptr);
	}

	void SwapChain::Present(GFX::Device& dev) const
	{
	}
}