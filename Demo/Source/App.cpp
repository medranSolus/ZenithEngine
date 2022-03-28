#include "App.h"
#include "Data/Camera.h"
#include "GFX/Primitive.h"

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
				case 'O':
				{
					if (engine.GetData().all_of<Data::RenderOutline>(cube))
						engine.GetData().remove<Data::RenderOutline>(cube);
					else
						engine.GetData().emplace<Data::RenderOutline>(cube);
					break;
				}
				case 'F':
				{
					if (engine.GetData().all_of<Data::RenderWireframe>(cube))
						engine.GetData().remove<Data::RenderWireframe>(cube);
					else
						engine.GetData().emplace<Data::RenderWireframe>(cube);
					break;
				}
				case 'L':
				{
					if (engine.GetData().all_of<Data::RenderLambertian>(cube))
						engine.GetData().remove<Data::RenderLambertian>(cube);
					else
						engine.GetData().emplace<Data::RenderLambertian>(cube);
					break;
				}
				case 'K':
				{
					if (engine.GetData().all_of<Data::ShadowCaster>(cube))
						engine.GetData().remove<Data::ShadowCaster>(cube);
					else
						engine.GetData().emplace<Data::ShadowCaster>(cube);
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
	engine.Reneder().UpdateWorldData(engine.Gfx().GetDevice(), camera);
	engine.EndFrame();
}

App::App(const std::string& commandLine)
	: engine({ WINDOW_TITLE, GfxApiType::DX12, 2, 0, 0, 10000, 800, { "Skybox/Space", ".png" } })
{
	camera = engine.GetData().create();
	Data::Camera camData =
	{
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f },
		{
			Math::ToRadians(60.0f),
			engine.Reneder().GetFrameRation(),
			0.01f, 1000.0f
		}
	};
	engine.GetData().emplace<Data::Camera>(camera, camData);
	engine.GetData().emplace<Data::TransformGlobal>(camera,
		engine.GetData().emplace<Data::Transform>(camera,
			Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f })));
	Float4x4 projection;
	Math::XMStoreFloat4x4(&projection, Math::XMMatrixPerspectiveFovLH(camData.Projection.FOV,
		camData.Projection.ViewRatio, camData.Projection.NearClip, camData.Projection.FarClip));
	engine.Reneder().UpdateSettingsData(engine.Gfx().GetDevice(), projection);

	EID cubeGeometry = engine.GetResourceData().create();
	EID cubeMaterial = engine.GetResourceData().create();
	engine.Gfx().GetDevice().BeginUploadRegion();
	{
		std::vector<U32> indices = GFX::Primitive::MakeCubeIndex();
		std::vector<GFX::Vertex> vertices = GFX::Primitive::MakeCubeVertex(indices);

		engine.GetResourceData().emplace<Math::BoundingBox>(cubeGeometry, GFX::Primitive::MakeCubeBoundingBox());
		Data::Geometry& geo = engine.GetResourceData().emplace<Data::Geometry>(cubeGeometry);
		geo.Vertices.Init(engine.Gfx().GetDevice(), { static_cast<U32>(vertices.size()), sizeof(GFX::Vertex), vertices.data() });
		geo.Indices.Init(engine.Gfx().GetDevice(), { static_cast<U32>(indices.size()), indices.data() });

		engine.GetResourceData().emplace<Data::MaterialPBR>(cubeMaterial, ColorF4(0.0f, 0.5f, 0.8f), ColorF3(0.5f, 0.5f, 0.5f), 0U, 1.0f, 0.5f, 1.0f);
		Data::MaterialBuffersPBR& mat = engine.GetResourceData().emplace<Data::MaterialBuffersPBR>(cubeMaterial);

		GFX::Resource::Texture::PackDesc desc;
		desc.Init(engine.GetTextureLibrary().Get(Data::MaterialPBR::TEX_SCHEMA_NAME));
		mat.Init(engine.Gfx().GetDevice(), engine.GetResourceData().get<Data::MaterialPBR>(cubeMaterial), desc);
		engine.Gfx().GetDevice().StartUpload();
	}
	{
		EID cube1 = engine.GetData().create();
		engine.GetData().emplace<Data::RenderLambertian>(cube1);
		engine.GetData().emplace<Data::RenderOutline>(cube1);
		engine.GetData().emplace<Data::ShadowCaster>(cube1);
		engine.GetData().emplace<Data::TransformGlobal>(cube1, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 3.0f }, { 1.0f, 1.0f, 1.0f }));
		engine.GetData().emplace<Data::MaterialID>(cube1, cubeMaterial);
		engine.GetData().emplace<Data::MeshID>(cube1, cubeGeometry);
	}
	{
		cube = engine.GetData().create();
		engine.GetData().emplace<Data::RenderLambertian>(cube);
		engine.GetData().emplace<Data::RenderOutline>(cube);
		engine.GetData().emplace<Data::RenderWireframe>(cube);
		engine.GetData().emplace<Data::ShadowCaster>(cube);
		Float4 rot;
		Math::XMStoreFloat4(&rot, Math::XMQuaternionRotationRollPitchYaw(0.0f, Math::ToRadians(45.0f), 0.0f));
		engine.GetData().emplace<Data::TransformGlobal>(cube, Data::Transform(rot, { 0.0f, 2.0f, 3.0f }, { 1.0f, 1.0f, 1.0f }));
		engine.GetData().emplace<Data::MaterialID>(cube, cubeMaterial);
		engine.GetData().emplace<Data::MeshID>(cube, cubeGeometry);
	}
	{
		EID lightBulb = engine.GetData().create();
		engine.GetData().emplace<Data::LightPoint>(lightBulb);
		engine.GetData().emplace<Data::TransformGlobal>(lightBulb, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { -20.0f, 2.0f, -4.0f }, { 1.0f, 1.0f, 1.0f }));
		Data::PointLight& pointLight = engine.GetData().emplace<Data::PointLight>(lightBulb, ColorF3(1.0f, 1.0f, 1.0f), 1.0f, ColorF3(0.05f, 0.05f, 0.05f));
		pointLight.SetAttenuationRange(50);
		engine.GetData().emplace<Data::PointLightBuffer>(lightBulb,
			Math::GetLightVolume(pointLight.Color, pointLight.Intensity, pointLight.AttnLinear, pointLight.AttnQuad),
			GFX::Resource::CBuffer()).Buffer.Init(engine.Gfx().GetDevice(), &pointLight, sizeof(Data::PointLight), false);
		engine.Gfx().GetDevice().StartUpload();
	}
	{
		EID nanosuit = engine.GetData().create();
		engine.GetData().emplace<Data::TransformGlobal>(nanosuit,
			engine.GetData().emplace<Data::Transform>(nanosuit, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, -8.2f, 6.0f }, { 0.7f, 0.7f, 0.7f })));
		Data::LoadGeometryFromModel(engine.Gfx().GetDevice(), engine.GetTextureLibrary(), engine.GetData(), engine.GetResourceData(), nanosuit, "Models/nanosuit/nanosuit.obj");
	}
	{
		EID bricks = engine.GetData().create();
		engine.GetData().emplace<Data::TransformGlobal>(bricks,
			engine.GetData().emplace<Data::Transform>(bricks, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { -5.0f, -2.0f, 7.0f }, { 2.0f, 2.0f, 2.0f })));
		Data::LoadGeometryFromModel(engine.Gfx().GetDevice(), engine.GetTextureLibrary(), engine.GetData(), engine.GetResourceData(), bricks, "Models/bricks/brick_wall.obj");
	}

	// Sample Scene
//#ifndef _ZE_MODE_DEBUG
#if 1
	{
		EID cam = engine.GetData().create();
		engine.GetData().emplace<Data::Camera>(cam,
			Data::Camera(Math::NormalizeReturn({ 0.5f, 0.0f, 0.5f }), { 0.0f, 1.0f, 0.0f },
				{
					Math::ToRadians(60.0f),
					engine.Reneder().GetFrameRation(),
					2.0f, 15.0f
				}));
		engine.GetData().emplace<Data::TransformGlobal>(cam,
			engine.GetData().emplace<Data::Transform>(cam,
				Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 40.0f, -4.0f }, { 1.0f, 1.0f, 1.0f })));
	}
	if (false)
	{
		EID pumpkinCandle = engine.GetData().create();
		engine.GetData().emplace<Data::LightPoint>(pumpkinCandle);
		engine.GetData().emplace<Data::TransformGlobal>(pumpkinCandle, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 14.0f, -6.3f, -5.0f }, { 1.0f, 1.0f, 1.0f }));
		Data::PointLight& pointLight = engine.GetData().emplace<Data::PointLight>(pumpkinCandle, ColorF3(1.0f, 0.96f, 0.27f), 5.0f, ColorF3(0.05f, 0.05f, 0.05f));
		pointLight.SetAttenuationRange(85);
		engine.GetData().emplace<Data::PointLightBuffer>(pumpkinCandle,
			Math::GetLightVolume(pointLight.Color, pointLight.Intensity, pointLight.AttnLinear, pointLight.AttnQuad),
			GFX::Resource::CBuffer()).Buffer.Init(engine.Gfx().GetDevice(), &pointLight, sizeof(Data::PointLight), false);
		engine.Gfx().GetDevice().StartUpload();
	}
	{
		EID blueIlumination = engine.GetData().create();
		engine.GetData().emplace<Data::LightPoint>(blueIlumination);
		engine.GetData().emplace<Data::TransformGlobal>(blueIlumination, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 43.0f, 27.0f, 1.8f }, { 1.0f, 1.0f, 1.0f }));
		Data::PointLight& pointLight = engine.GetData().emplace<Data::PointLight>(blueIlumination, ColorF3(0.0f, 0.46f, 1.0f), 10.0f, ColorF3(0.05f, 0.05f, 0.05f));
		pointLight.SetAttenuationRange(70);
		engine.GetData().emplace<Data::PointLightBuffer>(blueIlumination,
			Math::GetLightVolume(pointLight.Color, pointLight.Intensity, pointLight.AttnLinear, pointLight.AttnQuad),
			GFX::Resource::CBuffer()).Buffer.Init(engine.Gfx().GetDevice(), &pointLight, sizeof(Data::PointLight), false);
		engine.Gfx().GetDevice().StartUpload();
	}
	{
		EID torch = engine.GetData().create();
		engine.GetData().emplace<Data::LightPoint>(torch);
		engine.GetData().emplace<Data::TransformGlobal>(torch, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 21.95f, -1.9f, 9.9f }, { 1.0f, 1.0f, 1.0f }));
		Data::PointLight& pointLight = engine.GetData().emplace<Data::PointLight>(torch, ColorF3(1.0f, 0.0f, 0.2f), 5.0f, ColorF3(0.05f, 0.05f, 0.05f));
		pointLight.SetAttenuationRange(70);
		engine.GetData().emplace<Data::PointLightBuffer>(torch,
			Math::GetLightVolume(pointLight.Color, pointLight.Intensity, pointLight.AttnLinear, pointLight.AttnQuad),
			GFX::Resource::CBuffer()).Buffer.Init(engine.Gfx().GetDevice(), &pointLight, sizeof(Data::PointLight), false);
		engine.Gfx().GetDevice().StartUpload();
	}
	{
		EID spaceLight = engine.GetData().create();
		engine.GetData().emplace<Data::LightSpot>(spaceLight);
		engine.GetData().emplace<Data::TransformGlobal>(spaceLight, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 7.5f, 60.0f, -5.0f }, { 1.0f, 1.0f, 1.0f }));
		Data::SpotLight& spotLight = engine.GetData().emplace<Data::SpotLight>(spaceLight,
			ColorF3(1.3f, 2.3f, 1.3f), 8.0f, ColorF3(0.05f, 0.05f, 0.05f), Math::ToRadians(15.0f), Math::NormalizeReturn({ -0.64f, -1.0f, 0.5f }), Math::ToRadians(24.5f));
		spotLight.SetAttenuationRange(126);
		engine.GetData().emplace<Data::SpotLightBuffer>(spaceLight,
			Math::GetLightVolume(spotLight.Color, spotLight.Intensity, spotLight.AttnLinear, spotLight.AttnQuad),
			GFX::Resource::CBuffer()).Buffer.Init(engine.Gfx().GetDevice(), &spotLight, sizeof(Data::SpotLight), false);
		engine.Gfx().GetDevice().StartUpload();
	}
	{
		EID lionFlare = engine.GetData().create();
		engine.GetData().emplace<Data::LightSpot>(lionFlare);
		engine.GetData().emplace<Data::TransformGlobal>(lionFlare, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { -61.0f, -6.0f, 5.0f }, { 1.0f, 1.0f, 1.0f }));
		Data::SpotLight& spotLight = engine.GetData().emplace<Data::SpotLight>(lionFlare,
			ColorF3(0.8f, 0.0f, 0.8f), 9.0f, ColorF3(0.05f, 0.05f, 0.05f), Math::ToRadians(35.0f), Math::NormalizeReturn({ -1.0f, 1.0f, -0.7f }), Math::ToRadians(45.0f));
		spotLight.SetAttenuationRange(150);
		engine.GetData().emplace<Data::SpotLightBuffer>(lionFlare,
			Math::GetLightVolume(spotLight.Color, spotLight.Intensity, spotLight.AttnLinear, spotLight.AttnQuad),
			GFX::Resource::CBuffer()).Buffer.Init(engine.Gfx().GetDevice(), &spotLight, sizeof(Data::SpotLight), false);
		engine.Gfx().GetDevice().StartUpload();
	}
	{
		EID dragonFlame = engine.GetData().create();
		engine.GetData().emplace<Data::LightSpot>(dragonFlame);
		engine.GetData().emplace<Data::TransformGlobal>(dragonFlame, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { -35.0f, -8.0f, 2.0f }, { 1.0f, 1.0f, 1.0f }));
		Data::SpotLight& spotLight = engine.GetData().emplace<Data::SpotLight>(dragonFlame,
			ColorF3(0.04f, 0.0f, 0.52f), 9.0f, ColorF3(0.05f, 0.05f, 0.05f), Math::ToRadians(27.0f), Math::NormalizeReturn({ -0.6f, 0.75f, 0.3f }), Math::ToRadians(43.0f));
		spotLight.SetAttenuationRange(175);
		engine.GetData().emplace<Data::SpotLightBuffer>(dragonFlame,
			Math::GetLightVolume(spotLight.Color, spotLight.Intensity, spotLight.AttnLinear, spotLight.AttnQuad),
			GFX::Resource::CBuffer()).Buffer.Init(engine.Gfx().GetDevice(), &spotLight, sizeof(Data::SpotLight), false);
		engine.Gfx().GetDevice().StartUpload();
	}
	{
		EID moon = engine.GetData().create();
		engine.GetData().emplace<Data::LightDirectional>(moon);
		engine.GetData().emplace<Data::TransformGlobal>(moon, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { -35.0f, -8.0f, 2.0f }, { 1.0f, 1.0f, 1.0f }));
		Data::DirectionalLight& dirLight = engine.GetData().emplace<Data::DirectionalLight>(moon, ColorF3(0.7608f, 0.7725f, 0.8f), 0.1f, ColorF3(0.05f, 0.05f, 0.05f));
		engine.GetData().emplace<Data::Direction>(moon, Math::NormalizeReturn({ 0.0f, -0.7f, -0.7f }));
		engine.GetData().emplace<Data::DirectionalLightBuffer>(moon,
			GFX::Resource::CBuffer()).Buffer.Init(engine.Gfx().GetDevice(), &dirLight, sizeof(Data::DirectionalLight), false);
		engine.Gfx().GetDevice().StartUpload();
	}
	{
		EID sponza = engine.GetData().create();
		engine.GetData().emplace<Data::TransformGlobal>(sponza,
			engine.GetData().emplace<Data::Transform>(sponza, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, -8.0f, 0.0f }, { 0.045f, 0.045f, 0.045f })));
		Data::LoadGeometryFromModel(engine.Gfx().GetDevice(), engine.GetTextureLibrary(), engine.GetData(), engine.GetResourceData(), sponza, "Models/Sponza/sponza.obj");
	}
	if (false) // No UV generated
	{
		EID jackOLantern = engine.GetData().create();
		engine.GetData().emplace<Data::TransformGlobal>(jackOLantern,
			engine.GetData().emplace<Data::Transform>(jackOLantern, Data::Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 13.5f, -8.2f, -5.0f }, { 13.0f, 13.0f, 13.0f })));
		Data::LoadGeometryFromModel(engine.Gfx().GetDevice(), engine.GetTextureLibrary(), engine.GetData(), engine.GetResourceData(), jackOLantern, "Models/Jack/Jack_O_Lantern.3ds");
	}
	if (false) // Allocation bug
	{
		EID blackDragon = engine.GetData().create();
		engine.GetData().emplace<Data::TransformGlobal>(blackDragon,
			engine.GetData().emplace<Data::Transform>(blackDragon, Data::Transform(Math::GetQuaternion(0.0f, 290.0f, 0.0f), { -39.0f, -8.1f, 2.0f }, { 0.15f, 0.15f, 0.15f })));
		Data::LoadGeometryFromModel(engine.Gfx().GetDevice(), engine.GetTextureLibrary(), engine.GetData(), engine.GetResourceData(), blackDragon, "Models/Black Dragon/Dragon 2.5.fbx");
	}
	if (false) // Allocation bug
	{
		EID stingSword = engine.GetData().create();
		engine.GetData().emplace<Data::TransformGlobal>(stingSword,
			engine.GetData().emplace<Data::Transform>(stingSword, Data::Transform(Math::GetQuaternion(35.0f, 270.0f, 110.0f), { -20.0f, 0.0f, -6.0f }, { 0.2f, 0.2f, 0.2f })));
		Data::LoadGeometryFromModel(engine.Gfx().GetDevice(), engine.GetTextureLibrary(), engine.GetData(), engine.GetResourceData(), stingSword, "Models/Sting_Sword/Sting_Sword.obj");
	}
	if (false) // Allocation bug
	{
		EID tie = engine.GetData().create();
		engine.GetData().emplace<Data::TransformGlobal>(tie,
			engine.GetData().emplace<Data::Transform>(tie, Data::Transform(Math::GetQuaternion(0.0f, 87.1f, 301.0f), { 41.6f, 18.5f, 8.5f }, { 3.6f, 3.6f, 3.6f })));
		Data::LoadGeometryFromModel(engine.Gfx().GetDevice(), engine.GetTextureLibrary(), engine.GetData(), engine.GetResourceData(), tie, "Models/tie/tie.obj");
	}
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