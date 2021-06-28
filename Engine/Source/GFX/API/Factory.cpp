#include "GFX/API/Factory.h"
#include "GFX/API/DX/DX.h"
#include "GFX/API/DX11/DX11.h"

namespace ZE::GFX::API
{
	void Factory::InitGui(const Device& dev, const Context& ctx) const noexcept
	{
		switch (currentApi)
		{
		case ZE::GFX::API::Backend::DX11:
		{
			ImGui_ImplDX11_Init(((const DX11::Device&)(dev)).GetDevice(),
				((const DX11::Context&)(ctx)).GetContext());
			break;
		}
		case ZE::GFX::API::Backend::DX12:
		{
			assert("GUI not supported under DirectX12!" && false);
			break;
		}
		case ZE::GFX::API::Backend::Vulkan:
		{
			assert("GUI not supported under Vulkan!" && false);
			break;
		}
		}
	}

	void Factory::DisableGui() const noexcept
	{
		switch (currentApi)
		{
		case ZE::GFX::API::Backend::DX11:
		{
			ImGui_ImplDX11_Shutdown();
			break;
		}
		case ZE::GFX::API::Backend::DX12:
		{
			assert("GUI not supported under DirectX12!" && false);
			break;
		}
		case ZE::GFX::API::Backend::Vulkan:
		{
			assert("GUI not supported under Vulkan!" && false);
			break;
		}
		}
	}

	void Factory::StartGuiFrame() const noexcept
	{
		switch (currentApi)
		{
		case ZE::GFX::API::Backend::DX11:
		{
			ImGui_ImplDX11_NewFrame();
			break;
		}
		case ZE::GFX::API::Backend::DX12:
		{
			assert("GUI not supported under DirectX12!" && false);
			break;
		}
		case ZE::GFX::API::Backend::Vulkan:
		{
			assert("GUI not supported under Vulkan!" && false);
			break;
		}
		}
	}

	void Factory::EndGuiFrame() const noexcept
	{
		switch (currentApi)
		{
		case ZE::GFX::API::Backend::DX11:
		{
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			break;
		}
		case ZE::GFX::API::Backend::DX12:
		{
			assert("GUI not supported under DirectX12!" && false);
			break;
		}
		case ZE::GFX::API::Backend::Vulkan:
		{
			assert("GUI not supported under Vulkan!" && false);
			break;
		}
		}
	}

	Context* Factory::MakeContext(Device& dev, bool deffered)
	{
		switch (currentApi)
		{
		case ZE::GFX::API::Backend::DX11:
			return new DX11::Context((DX11::Device&)(dev), deffered);
		case ZE::GFX::API::Backend::DX12:
			assert("No support for Context under DirectX12!" && false);
		case ZE::GFX::API::Backend::Vulkan:
			assert("No support for Context under Vulkan!" && false);
		}
		return nullptr;
	}

	Device* Factory::MakeDevice()
	{
		switch (currentApi)
		{
		case ZE::GFX::API::Backend::DX11:
			return new DX11::Device();
		case ZE::GFX::API::Backend::DX12:
			assert("No support for Device under DirectX12!" && false);
		case ZE::GFX::API::Backend::Vulkan:
			assert("No support for Device under Vulkan!" && false);
		}
		return nullptr;
	}

	GPerf* Factory::MakeGpuPerf(Device& dev)
	{
		switch (currentApi)
		{
		case ZE::GFX::API::Backend::DX11:
			return new DX11::GPerf((DX11::Device&)(dev));
		case ZE::GFX::API::Backend::DX12:
			assert("No support for GPerf under DirectX12!" && false);
		case ZE::GFX::API::Backend::Vulkan:
			assert("No support for GPerf under Vulkan!" && false);
		}
		return nullptr;
	}

	SwapChain* Factory::MakeSwapChain(const Window::MainWindow& window, Device& dev)
	{
		switch (currentApi)
		{
		case ZE::GFX::API::Backend::DX11:
			return new DX11::SwapChain(window, (DX11::Device&)(dev));
		case ZE::GFX::API::Backend::DX12:
			assert("No support for SwapChain under DirectX12!" && false);
		case ZE::GFX::API::Backend::Vulkan:
			assert("No support for SwapChain under Vulkan!" && false);
		}
		return nullptr;
	}
}