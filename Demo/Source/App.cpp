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
	Data::Camera camData =
	{
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f },
		{
			Math::ToRadians(60.0f),
			static_cast<float>(engine.Reneder().GetFrameWidth()) / static_cast<float>(engine.Reneder().GetFrameHeight()),
			0.001f, 1000.0f
		}
	};
	scene.AddCamera(camera, camData);
	scene.AddTransform(camera, { { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } });
	scene.CurrentProjection = Math::XMMatrixPerspectiveFovLH(camData.Projection.FOV, camData.Projection.ScreenRatio, camData.Projection.NearClip, camData.Projection.FarClip);
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