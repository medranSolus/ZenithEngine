#include "Engine.h"

namespace ZE
{
	Engine::Engine(const char* windowName, GFX::API::Backend gfxApi, U32 width, U32 height)
		: window(windowName, width, height)
	{
		Settings::Init(gfxApi);
		graphics.Init(window);
		Settings::GetGfxApi().InitGui(graphics.GetDevice(), graphics.GetMainContext());
	}

	Engine::~Engine()
	{
		Settings::GetGfxApi().DisableGui();
	}

	void Engine::BeginFrame() const
	{
		if (IsGuiActive())
		{
			Settings::GetGfxApi().StartGuiFrame();
			window.NewGuiFrame();
			ImGui::NewFrame();
		}
	}

	void Engine::EndFrame() const
	{
		if (IsGuiActive())
		{
			ImGui::Render();
			Settings::GetGfxApi().EndGuiFrame();
		}
		graphics.Present();
	}
}