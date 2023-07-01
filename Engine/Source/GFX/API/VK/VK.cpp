#include "GFX/API/VK/VulkanException.h"

namespace ZE::GFX::API::VK
{
	VkSurfaceKHR CreateSurface(const Window::MainWindow& window, VkInstance instance)
	{
		ZE_VK_ENABLE();
		VkSurfaceKHR surface = VK_NULL_HANDLE;

#if _ZE_PLATFORM_WINDOWS
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr };
		surfaceCreateInfo.flags = 0;
		surfaceCreateInfo.hinstance = Window::MainWindow::GetInstance();
		surfaceCreateInfo.hwnd = window.GetHandle();

		ZE_VK_THROW_NOSUCC(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface));
#else
#	error Unsupported platform for Vulkan SwapChain implementation!
#endif
		return surface;
	}
}