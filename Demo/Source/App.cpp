#include "App.h"
#include "Data/Camera.h"
#include "GFX/Primitive.h"

void App::ProcessInput()
{
	Window::MainWindow& window = engine.Window();
	while (window.Mouse().IsInput())
	{
		if (auto opt = window.Mouse().Read())
		{
			const auto& value = opt.value();
			if (value.IsRightDown() && window.IsCursorEnabled())
				Data::RotateCamera(engine.GetData(), currentCamera,
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
					Data::RotateCamera(engine.GetData(), currentCamera,
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
		Data::MoveCameraZ(engine.GetData(), currentCamera, moveSpeed, cameraType);
	if (keyboard.IsKeyDown('S'))
		Data::MoveCameraZ(engine.GetData(), currentCamera, -moveSpeed, cameraType);
	if (keyboard.IsKeyDown('A'))
		Data::MoveCameraX(engine.GetData(), currentCamera, -moveSpeed, cameraType);
	if (keyboard.IsKeyDown('D'))
		Data::MoveCameraX(engine.GetData(), currentCamera, moveSpeed, cameraType);
	if (keyboard.IsKeyDown(VK_SPACE))
		Data::MoveCameraY(engine.GetData(), currentCamera, moveSpeed, cameraType);
	if (keyboard.IsKeyDown('C'))
		Data::MoveCameraY(engine.GetData(), currentCamera, -moveSpeed, cameraType);
	if (keyboard.IsKeyDown('Q'))
		Data::RollCamera(engine.GetData(), currentCamera, rollSpeed, cameraType);
	if (keyboard.IsKeyDown('E'))
		Data::RollCamera(engine.GetData(), currentCamera, -rollSpeed, cameraType);
}

void App::ShowOptionsWindow()
{
	if (ImGui::Begin("Options"/*, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize*/))
	{
		if (ImGui::Button("Exit"))
			run = false;
		ImGui::SameLine();
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		engine.Reneder().ShowWindow(engine.Gfx().GetDevice());
	}
	ImGui::End();
}

void App::ShowObjectWindow()
{
	if (ImGui::Begin("Objects"/*, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize*/))
	{
	}
	ImGui::End();
}

EID App::AddCamera(std::string&& name, float nearZ, float farZ, float fov,
	Float3&& position, const Float3& angle)
{
	EID camera = engine.GetData().create();
	engine.GetData().emplace<std::string>(camera, std::move(name));

	const Vector rotor = Math::XMQuaternionRotationRollPitchYaw(Math::ToRadians(angle.x), Math::ToRadians(angle.y), Math::ToRadians(angle.z));

	Float4 roation;
	Float3 eyeVector, upVector;
	XMStoreFloat3(&eyeVector, Math::XMVector3Rotate({ 0.0f, 0.0f, 1.0f, 0.0f }, rotor));
	XMStoreFloat3(&upVector, Math::XMVector3Rotate({ 0.0f, 1.0f, 0.0f, 0.0f }, rotor));
	XMStoreFloat4(&roation, rotor);

	engine.GetData().emplace<Data::Camera>(camera,
		Data::Camera(std::move(eyeVector), std::move(upVector),
			{
				Math::ToRadians(60.0f),
				engine.Reneder().GetFrameRation(),
				nearZ, farZ
			}));

	engine.GetData().emplace<Data::TransformGlobal>(camera,
		engine.GetData().emplace<Data::Transform>(camera,
			std::move(roation), std::move(position), Math::UnitScale()));

	return camera;
}

EID App::AddModel(std::string&& name, Float3&& position,
	const Float3& angle, float scale, const std::string& file)
{
	EID model = engine.GetData().create();
	engine.GetData().emplace<std::string>(model, std::move(name));

	engine.GetData().emplace<Data::TransformGlobal>(model,
		engine.GetData().emplace<Data::Transform>(model,
			Math::GetQuaternion(angle.x, angle.y, angle.z), std::move(position), Float3(scale, scale, scale)));

	Data::LoadGeometryFromModel(engine.Gfx().GetDevice(), engine.GetTextureLibrary(),
		engine.GetData(), engine.GetResourceData(), model, file);
	return model;
}

EID App::AddPointLight(std::string&& name, Float3&& position,
	ColorF3&& color, float intensity, U64 range)
{
	EID light = engine.GetData().create();
	engine.GetData().emplace<std::string>(light, std::move(name));
	engine.GetData().emplace<Data::LightPoint>(light);

	engine.GetData().emplace<Data::TransformGlobal>(light,
		engine.GetData().emplace<Data::Transform>(light, Math::NoRotation(), std::move(position), Math::UnitScale()));

	Data::PointLight& pointLight = engine.GetData().emplace<Data::PointLight>(light, std::move(color), intensity, ColorF3(0.05f, 0.05f, 0.05f));
	pointLight.SetAttenuationRange(range);

	Data::PointLightBuffer& buffer = engine.GetData().emplace<Data::PointLightBuffer>(light,
		Math::GetLightVolume(pointLight.Color, pointLight.Intensity, pointLight.AttnLinear, pointLight.AttnQuad));
	buffer.Buffer.Init(engine.Gfx().GetDevice(), &pointLight, sizeof(Data::PointLight), false);

	engine.Gfx().GetDevice().StartUpload();
	return light;
}

EID App::AddSpotLight(std::string&& name, Float3&& position,
	ColorF3&& color, float intensity, U64 range,
	float innerAngle, float outerAngle, const Float3& direction)
{
	EID light = engine.GetData().create();
	engine.GetData().emplace<std::string>(light, std::move(name));
	engine.GetData().emplace<Data::LightSpot>(light);

	engine.GetData().emplace<Data::TransformGlobal>(light,
		engine.GetData().emplace<Data::Transform>(light,
			Math::NoRotation(), std::move(position), Math::UnitScale()));

	Data::SpotLight& spotLight = engine.GetData().emplace<Data::SpotLight>(light,
		std::move(color), intensity, ColorF3(0.05f, 0.05f, 0.05f), Math::ToRadians(innerAngle),
		Math::NormalizeReturn(direction), Math::ToRadians(outerAngle));
	spotLight.SetAttenuationRange(range);

	Data::SpotLightBuffer& buffer = engine.GetData().emplace<Data::SpotLightBuffer>(light,
		Math::GetLightVolume(spotLight.Color, spotLight.Intensity, spotLight.AttnLinear, spotLight.AttnQuad));
	buffer.Buffer.Init(engine.Gfx().GetDevice(), &spotLight, sizeof(Data::SpotLight), false);

	engine.Gfx().GetDevice().StartUpload();
	return light;
}

EID App::AddDirectionalLight(std::string&& name,
	ColorF3&& color, float intensity, const Float3& direction)
{
	EID light = engine.GetData().create();
	engine.GetData().emplace<std::string>(light, std::move(name));
	engine.GetData().emplace<Data::LightDirectional>(light);

	Data::DirectionalLight& dirLight = engine.GetData().emplace<Data::DirectionalLight>(light,
		std::move(color), intensity, ColorF3(0.05f, 0.05f, 0.05f));
	engine.GetData().emplace<Data::Direction>(light, Math::NormalizeReturn(direction));

	Data::DirectionalLightBuffer& buffer = engine.GetData().emplace<Data::DirectionalLightBuffer>(light);
	buffer.Buffer.Init(engine.Gfx().GetDevice(), &dirLight, sizeof(Data::DirectionalLight), false);

	engine.Gfx().GetDevice().StartUpload();
	return light;
}

void App::MakeFrame()
{
	engine.BeginFrame();
	//ImGui::ShowDemoWindow();
	ShowOptionsWindow();
	ShowObjectWindow();
	engine.Reneder().UpdateWorldData(engine.Gfx().GetDevice(), currentCamera);
	engine.EndFrame();
}

App::App(const std::string& commandLine)
	: engine({ WINDOW_TITLE, GfxApiType::DX12, 2, 0, 0, 10000, 800, { "Skybox/Space", ".png" } })
{
	engine.Gui().SetFont("Fonts/Arial.ttf", 14.0f);

	currentCamera = AddCamera("Main camera", 0.01f, 1000.0f, 60.0f, Math::StartPosition(), Math::NoRotationAngles());
	Data::Camera& camData = engine.GetData().get<Data::Camera>(currentCamera);
	Float4x4 projection;
	Math::XMStoreFloat4x4(&projection, Math::XMMatrixPerspectiveFovLH(camData.Projection.FOV,
		camData.Projection.ViewRatio, camData.Projection.NearClip, camData.Projection.FarClip));
	engine.Reneder().UpdateSettingsData(engine.Gfx().GetDevice(), projection);

	engine.Gfx().GetDevice().BeginUploadRegion();
	AddPointLight("Light bulb", { -20.0f, 2.0f, -4.0f }, { 1.0f, 1.0f, 1.0f }, 1.0f, 50);
	AddModel("Nanosuit", { 0.0f, -8.2f, 6.0f }, Math::NoRotationAngles(), 0.7f, "Models/nanosuit/nanosuit.obj");
	AddModel("Brick wall", { -5.0f, -2.0f, 7.0f }, Math::NoRotationAngles(), 2.0f, "Models/bricks/brick_wall.obj");

	// Sample Scene
//#ifndef _ZE_MODE_DEBUG
#if 1
	AddCamera("Camera #2", 2.0f, 15.0f, 60.0f, { 0.0f, 40.0f, -4.0f }, { 0.0f, 45.0f, 0.0f });
	if (false)
		AddPointLight("Pumpkin candle", { 14.0f, -6.3f, -5.0f }, { 1.0f, 0.96f, 0.27f }, 5.0f, 85);
	AddPointLight("Blue ilumination", { 43.0f, 27.0f, 1.8f }, { 0.0f, 0.46f, 1.0f }, 10.0f, 70);
	AddPointLight("Torch", { 21.95f, -1.9f, 9.9f }, { 1.0f, 0.0f, 0.2f }, 5.0f, 70);

	AddSpotLight("Space light", { 7.5f, 60.0f, -5.0f }, { 1.3f, 2.3f, 1.3f }, 8.0f, 126, 15.0f, 24.5f, { -0.64f, -1.0f, 0.5f });
	AddSpotLight("Lion flare", { -61.0f, -6.0f, 5.0f }, { 0.8f, 0.0f, 0.8f }, 9.0f, 150, 35.0f, 45.0f, { -1.0f, 1.0f, -0.7f });
	AddSpotLight("Dragon flame", { -35.0f, -8.0f, 2.0f }, { 0.04f, 0.0f, 0.52f }, 9.0f, 175, 27.0f, 43.0f, { -0.6f, 0.75f, 0.3 });

	AddDirectionalLight("Moon", { 0.7608f, 0.7725f, 0.8f }, 0.1f, { 0.0f, -0.7f, -0.7f });

	AddModel("Sponza", { 0.0f, -8.0f, 0.0f }, Math::NoRotationAngles(), 0.045f, "Models/Sponza/sponza.obj");
	if (false) // No UV generated
		AddModel("Jack'O'Lantern", { 13.5f, -8.2f, -5.0f }, Math::NoRotationAngles(), 13.0f, "Models/Jack/Jack_O_Lantern.3ds");
	if (false) // Allocation bug
		AddModel("Black dragon", { -39.0f, -8.1f, 2.0f }, { 0.0f, 290.0f, 0.0f }, 0.15f, "Models/Black Dragon/Dragon 2.5.fbx");
	if (false) // Allocation bug
		AddModel("Sting sword", { -20.0f, 0.0f, -6.0f }, { 35.0f, 270.0f, 110.0f }, 0.2f, "Models/Sting_Sword/Sting_Sword.obj");
	if (false) // Allocation bug
		AddModel("TIE", { 41.6f, 18.5f, 8.5f }, { 0.0f, 87.1f, 301.0f }, 3.6f, "Models/tie/tie.obj");
#endif
	engine.Gfx().GetDevice().EndUploadRegion();
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