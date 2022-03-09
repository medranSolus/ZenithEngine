#include "App.h"

void App::ProcessInput()
{
	while (engine.Window().Keyboard().IsKeyReady())
	{
		if (auto opt = engine.Window().Keyboard().ReadKey())
		{
			if (opt->IsDown())
			{
				switch (opt->GetCode())
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
	ImGui::ShowDemoWindow();
	engine.Reneder().UpdateWorldData(engine.Gfx().GetDevice(), camera);
	engine.EndFrame();
}

App::App(const std::string& commandLine)
	: engine({ WINDOW_TITLE, GfxApiType::DX12, 2, 0, 0, 10000, 8000, { "Skybox/Space", ".png" } })
{
	engine.Reneder().SetActiveScene(scene);
	camera = scene.CreateEntity();
	scene.AddCamera(camera, {});
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
		scene.UpdateTransforms();
		MakeFrame();
	}
	return 0;
}