#include "App.h"

void App::ProcessInput()
{
	while (engine.Window().Keyboard().IsKeyReady())
	{
		if (auto opt = engine.Window().Keyboard().ReadKey())
		{
			if (opt.value().IsDown())
			{
				switch (opt.value().GetCode())
				{
				case VK_ESCAPE:
				{
					engine.Window().SwitchCursor();
					break;
				}
				case VK_F1:
				{
					engine.ToggleGui();
					break;
				}
				}
			}
		}
	}
}

void App::MakeFrame()
{
	engine.BeginFrame();
	//ImGui::ShowDemoWindow();
	engine.EndFrame();
}

App::App(const std::string& commandLine)
	: engine(WINDOW_TITLE, GfxApiType::DX12, 1600, 896)
{
}

int App::Run()
{
	std::pair<bool, int> status;
	while (run)
	{
		status = engine.Window().ProcessMessage();
		if (status.first)
			return status.second;
		ProcessInput();
		MakeFrame();
	}
	return 0;
}