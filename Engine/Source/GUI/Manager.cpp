#include "GUI/Manager.h"
#include "WarningGuardOn.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_dx12.h"
#include "WarningGuardOff.h"

namespace ZE::GUI
{
	Manager::Manager()
	{
		if (!std::filesystem::exists("imgui.ini") && std::filesystem::exists("imgui_default.ini"))
			std::filesystem::copy_file("imgui_default.ini", "imgui.ini");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		auto& style = ImGui::GetStyle();
		style.WindowRounding = 1;
		style.WindowBorderSize = 1;
		style.Colors[ImGuiCol_WindowBg].w = 0.785f;
	}

	void Manager::Init(GFX::Device& dev) const noexcept
	{
		switch (Settings::GetGfxApi())
		{
		case GfxApiType::DX11:
		{
			ImGui_ImplDX11_Init(dev.Get().dx11.GetDevice(), dev.Get().dx11.GetMainContext());
			break;
		}
		case GfxApiType::DX12:
		{
			auto handles = dev.Get().dx12.AddStaticDescs(1);
			ImGui_ImplDX12_Init(dev.Get().dx12.GetDevice(), static_cast<int>(Settings::GetBackbufferCount()),
				GFX::API::DX::GetDXFormat(Settings::GetBackbufferFormat()),
				dev.Get().dx12.GetDescHeap(), handles.first, handles.second);
			return;
		}
		default:
		{
			ZE_ASSERT(false, "GUI not supported under current API!");
			break;
		}
		}
	}

	void Manager::Disable() const noexcept
	{
		switch (Settings::GetGfxApi())
		{
		case GfxApiType::DX11:
		{
			ImGui_ImplDX11_Shutdown();
			break;
		}
		case GfxApiType::DX12:
		{
			ImGui_ImplDX12_Shutdown();
			return;
		}
		default:
		{
			assert("GUI not supported under current API!" && false);
			break;
		}
		}
	}

	void Manager::StartFrame(const Window::MainWindow& window) const noexcept
	{
		switch (Settings::GetGfxApi())
		{
		case GfxApiType::DX11:
		{
			ImGui_ImplDX11_NewFrame();
			break;
		}
		case GfxApiType::DX12:
		{
			ImGui_ImplDX12_NewFrame();
			break;
		}
		default:
		{
			ZE_ASSERT(false, "GUI not supported under current API!");
			break;
		}
		}
		window.NewGuiFrame();
		ImGui::NewFrame();
	}

	void Manager::EndFrame(GFX::Graphics& gfx) const noexcept
	{
		ImGui::Render();
		auto& mainList = gfx.GetMainList();
		switch (Settings::GetGfxApi())
		{
		case GfxApiType::DX11:
		{
			ZE_DRAW_TAG_BEGIN(mainList.Get().dx11, L"ImGui", PixelVal::Cobalt);
			mainList.Get().dx11.GetContext()->OMSetRenderTargets(1,
				reinterpret_cast<ID3D11RenderTargetView* const*>(gfx.GetSwapChain().Get().dx11.GetRTV().GetAddressOf()), nullptr);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			ZE_DRAW_TAG_END(mainList.Get().dx11);
			break;
		}
		case GfxApiType::DX12:
		{
			auto& dev = gfx.GetDevice().Get().dx12;
			auto& swapChain = gfx.GetSwapChain().Get().dx12;
			mainList.Get().dx12.Open(dev);
			ZE_DRAW_TAG_BEGIN(mainList.Get().dx12, L"ImGui", PixelVal::Cobalt);

			const D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChain.GetCurrentRTV();
			mainList.Get().dx12.GetList()->OMSetRenderTargets(1, &rtv, true, nullptr);
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mainList.Get().dx12.GetList());
			mainList.Get().dx12.GetList()->ResourceBarrier(1, &swapChain.GetPresentBarrier());

			ZE_DRAW_TAG_END(mainList.Get().dx12);
			mainList.Get().dx12.Close(gfx.GetDevice());
			dev.ExecuteMain(mainList);
			return;
		}
		default:
		{
			ZE_ASSERT(false, "GUI not supported under current API!");
			break;
		}
		}
	}

	void Manager::SetFont(const std::string& font, float size) const
	{
		ImFontAtlas* atlas = ImGui::GetIO().Fonts;
		if (atlas->Fonts.size())
			atlas->Clear();
		atlas->AddFontFromFileTTF(font.c_str(), size);
	}
}