#include "App.h"
#include "Data/Operations.h"

void App::ShowOptionsWindow()
{
	if (ImGui::Begin("Options"/*, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize*/))
	{
		if (ImGui::Button("Exit"))
			run = false;
		ImGui::SameLine();
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		const Float3& pos = engine.GetData().get<Data::Transform>(camera).Position;
		ImGui::Text("Postion [ %.1f | %.1f | %.1f ]", pos.x, pos.y, pos.z);
	}
	ImGui::End();
}

void App::ProcessInput()
{
	Window::MainWindow& window = engine.Window();
	while (window.Mouse().IsInput())
	{
		if (auto opt = window.Mouse().Read())
		{
			const auto& value = opt.value();
			if (value.IsRightDown() && window.IsCursorEnabled())
				Data::RotateCamera(engine.GetData(), camera,
					rotateSpeed * static_cast<float>(value.GetDY()) / static_cast<float>(engine.Reneder().GetFrameHeight()),
					rotateSpeed * static_cast<float>(value.GetDX()) / static_cast<float>(engine.Reneder().GetFrameHeight()), cameraType);

			switch (value.GetType())
			{
			case Window::Mouse::Event::Type::WheelForward:
			{
				if (!window.IsCursorEnabled() && moveSpeed <= MAX_MOVE_SPEED - 0.01f - FLT_EPSILON)
					moveSpeed += 0.01f;
				break;
			}
			case Window::Mouse::Event::Type::WheelBackward:
			{
				if (!window.IsCursorEnabled() && moveSpeed >= 0.012f + FLT_EPSILON)
					moveSpeed -= 0.01f;
				break;
			}
			case Window::Mouse::Event::Type::RawMove:
			{
				if (!window.IsCursorEnabled())
					Data::RotateCamera(engine.GetData(), camera,
						rotateSpeed * static_cast<float>(value.GetDY()) / static_cast<float>(engine.Reneder().GetFrameHeight()),
						rotateSpeed * static_cast<float>(value.GetDX()) / static_cast<float>(engine.Reneder().GetFrameHeight()), cameraType);
				break;
			}
			default:
				break;
			}
		}
	}

	Window::Keyboard& keyboard = window.Keyboard();
	while (keyboard.IsKeyReady())
	{
		if (auto opt = keyboard.ReadKey())
		{
			if (opt->IsDown())
			{
				switch (opt->GetCode())
				{
				case VK_ESCAPE:
				{
					window.SwitchCursor();
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

	if (keyboard.IsKeyDown('W'))
		Data::MoveCameraZ(engine.GetData(), camera, moveSpeed, cameraType);
	if (keyboard.IsKeyDown('S'))
		Data::MoveCameraZ(engine.GetData(), camera, -moveSpeed, cameraType);
	if (keyboard.IsKeyDown('A'))
		Data::MoveCameraX(engine.GetData(), camera, -moveSpeed, cameraType);
	if (keyboard.IsKeyDown('D'))
		Data::MoveCameraX(engine.GetData(), camera, moveSpeed, cameraType);
	if (keyboard.IsKeyDown(VK_SPACE))
		Data::MoveCameraY(engine.GetData(), camera, moveSpeed, cameraType);
	if (keyboard.IsKeyDown('C'))
		Data::MoveCameraY(engine.GetData(), camera, -moveSpeed, cameraType);
	if (keyboard.IsKeyDown('Q'))
		Data::RollCamera(engine.GetData(), camera, rollSpeed, cameraType);
	if (keyboard.IsKeyDown('E'))
		Data::RollCamera(engine.GetData(), camera, -rollSpeed, cameraType);
}

void App::MakeFrame()
{
	engine.BeginFrame();
	ImGui::ShowDemoWindow();
	ShowOptionsWindow();
	engine.Reneder().UpdateWorldData(engine.Gfx().GetDevice(), camera, currentProjection);
	engine.EndFrame();
}

App::App(const std::string& commandLine)
	: engine({ WINDOW_TITLE, GfxApiType::DX12, 2, 0, 0, 10000, 8000, { "Skybox/Space", ".png" } })
{
	camera = engine.GetData().create();
	Data::Camera camData =
	{
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f },
		{
			Math::ToRadians(60.0f),
			engine.Reneder().GetFrameRation(),
			0.001f, 1000.0f
		}
	};
	engine.GetData().emplace<Data::Camera>(camera, camData);
	engine.GetData().emplace<Data::Transform>(camera, Data::Transform({ 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
	currentProjection = Math::XMMatrixPerspectiveFovLH(camData.Projection.FOV, camData.Projection.ViewRatio, camData.Projection.NearClip, camData.Projection.FarClip);
}

int App::Run()
{
	while (run)
	{
		const std::pair<bool, int> status = engine.Window().ProcessMessage();
		if (status.first)
			return status.second;
		ProcessInput();
		//scene.UpdateTransforms();
		MakeFrame();
	}
	return 0;
}