#include "GUI/ImGuiManager.h"
#include "GFX/ImGuiBackendData.h"

namespace ZE::GUI
{
	ImGuiManager::ImGuiManager() noexcept
	{
		std::error_code ec = {};
		bool configExists = std::filesystem::exists("imgui.ini", ec);
		if (ec)
		{
			ZE_CODE_WARNING(ec, "Cannot check for presence of \"imgui.ini\" file, falling back to default config.");
			configExists = false;
			ec.clear();
		}
		if (!configExists)
		{
			configExists = std::filesystem::exists("imgui_default.ini");
			if (ec)
			{
				ZE_CODE_ERROR(ec, "Cannot check for presence of \"imgui_default.ini\" file, ImGui windows may be invalid!");
			}
			else if (configExists)
			{
				configExists = std::filesystem::copy_file("imgui_default.ini", "imgui.ini", std::filesystem::copy_options::overwrite_existing, ec);
				if (ec || !configExists)
				{
					ZE_CODE_ERROR(ec, "Could not copyy \"imgui_default.ini\" to active config file, ImGui windows may be invalid!");
				}
			}
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_IsSRGB;
		ImGui::StyleColorsDark();
		auto& style = ImGui::GetStyle();
		style.WindowRounding = 1;
		style.WindowBorderSize = 1;
		style.Colors[ImGuiCol_WindowBg].w = 0.785f;
	}

	void ImGuiManager::StartFrame(const Window::MainWindow& window) const noexcept
	{
		GFX::ImGuiBackendData::NewFrame();
		window.NewImGuiFrame();
		ImGui::NewFrame();
	}

	void ImGuiManager::SetFont(std::string_view font, float size) const noexcept
	{
		ImFontAtlas* atlas = ImGui::GetIO().Fonts;
		if (atlas->Fonts.size())
		{
			atlas->Clear();
			GFX::ImGuiBackendData::RecreateFonts();
		}
		if (!atlas->AddFontFromFileTTF(font.data(), size))
			Logger::Warning("Failed to load new font for ImGui: " + std::string(font));
	}
}