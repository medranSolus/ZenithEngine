#pragma once
#include <string>

namespace GFX
{
	class GUIManager
	{
	public:
		GUIManager();
		~GUIManager();
		
		void SetFont(const std::string & font, float size);
	};
}