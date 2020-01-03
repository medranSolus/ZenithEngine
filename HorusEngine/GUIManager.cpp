#include "GUIManager.h"

namespace GFX
{
	GUIManager::GUIManager()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
	}

	void GUIManager::SetFont(const std::string & font, float size)
	{
		ImFontAtlas * atlas = ImGui::GetIO().Fonts;
		if (atlas->Fonts.size())
			atlas->Clear();
		atlas->AddFontFromFileTTF(font.c_str(), size);
	}
}
