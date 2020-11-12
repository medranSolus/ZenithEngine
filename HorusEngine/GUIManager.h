#pragma once
#include "ImGui/imgui.h"
#include <string>

namespace GFX::GUI
{
	class GUIManager
	{
	public:
		GUIManager();
		GUIManager(const GUIManager&) = delete;
		GUIManager& operator=(const GUIManager&) = delete;
		inline ~GUIManager() { ImGui::DestroyContext(); }

		void SetFont(const std::string& font, float size);
	};
}