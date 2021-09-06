#include "GUI/Manager.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_dx12.h"
#include <filesystem>

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
			return;
		}
		default:
		{
			assert("GUI not supported under current API!" && false);
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
			return;
		}
		default:
		{
			assert("GUI not supported under current API!" && false);
			break;
		}
		}
		window.NewGuiFrame();
		ImGui::NewFrame();
	}

	void Manager::EndFrame() const noexcept
	{
		ImGui::Render();
		switch (Settings::GetGfxApi())
		{
		case GfxApiType::DX11:
		{
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			break;
		}
		case GfxApiType::DX12:
		{
			return;
		}
		default:
		{
			assert("GUI not supported under current API!" && false);
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