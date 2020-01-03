#pragma once
#include <string>
#include "ImGui/imgui.h"

namespace GFX
{
	class GUIManager
	{
	public:
		GUIManager();
		inline ~GUIManager() { ImGui::DestroyContext(); }
		
		void SetFont(const std::string & font, float size);
	};
}