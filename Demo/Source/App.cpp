#include "App.h"
#include "Data/Operations.h"
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
			0.01f, 1000.0f
		}
	};
	engine.GetData().emplace<Data::Camera>(camera, camData);
	engine.GetData().emplace<Data::Transform>(camera, Data::Transform({ 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
	currentProjection = Math::XMMatrixPerspectiveFovLH(camData.Projection.FOV, camData.Projection.ViewRatio, camData.Projection.NearClip, camData.Projection.FarClip);

	std::vector<U32> indices = GFX::Primitive::MakeCubeIndex();
	std::vector<GFX::Vertex> vertices = GFX::Primitive::MakeCubeVertex(indices);
	engine.Gfx().GetDevice().BeginUploadRegion();
	{
		EID cube1 = engine.GetData().create();
		engine.GetData().emplace<Data::RenderLambertian>(cube1);
		engine.GetData().emplace<Data::RenderOutline>(cube1);
		engine.GetData().emplace<Data::ShadowCaster>(cube1);
		engine.GetData().emplace<Data::TransformGlobal>(cube1, Data::Transform({ 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 3.0f }, { 1.0f, 1.0f, 1.0f }));
		engine.GetData().emplace<Data::MaterialPBR>(cube1, ColorF4(0.0f, 0.5f, 0.8f), ColorF3(0.5f, 0.5f, 0.5f), 0U, 1.0f, 0.5f, 1.0f);
		Data::Geometry& geo = engine.GetData().emplace<Data::Geometry>(cube1, GFX::Resource::VertexBuffer(), GFX::Resource::IndexBuffer());
		Data::MaterialBuffersPBR& mat = engine.GetData().emplace<Data::MaterialBuffersPBR>(cube1, Data::MaterialBuffersPBR());

		GFX::Resource::Texture::PackDesc desc;
		std::vector<GFX::Surface> surfaces;
		desc.AddTexture(GFX::Resource::Texture::Type::Tex2D, GFX::Resource::Texture::Usage::PixelShader, std::move(surfaces));
		desc.AddTexture(GFX::Resource::Texture::Type::Tex2D, GFX::Resource::Texture::Usage::PixelShader, std::move(surfaces));
		desc.AddTexture(GFX::Resource::Texture::Type::Tex2D, GFX::Resource::Texture::Usage::PixelShader, std::move(surfaces));
		desc.AddTexture(GFX::Resource::Texture::Type::Tex2D, GFX::Resource::Texture::Usage::PixelShader, std::move(surfaces));
		mat.Init(engine.Gfx().GetDevice(), engine.GetData().get<Data::MaterialPBR>(cube1), desc);

		geo.Vertices.Init(engine.Gfx().GetDevice(), { static_cast<U32>(vertices.size()), sizeof(GFX::Vertex), vertices.data() });
		geo.Indices.Init(engine.Gfx().GetDevice(), { static_cast<U32>(indices.size()), indices.data() });
	}
	cube = engine.GetData().create();
	engine.GetData().emplace<Data::RenderLambertian>(cube);
	engine.GetData().emplace<Data::RenderOutline>(cube);
	engine.GetData().emplace<Data::RenderWireframe>(cube);
	engine.GetData().emplace<Data::ShadowCaster>(cube);
	Float4 rot;
	Math::XMStoreFloat4(&rot, Math::XMQuaternionRotationRollPitchYaw(0.0f, Math::ToRadians(45.0f), 0.0f));
	engine.GetData().emplace<Data::TransformGlobal>(cube, Data::Transform(rot, { 0.0f, 2.0f, 3.0f }, { 1.0f, 1.0f, 1.0f }));
	engine.GetData().emplace<Data::MaterialPBR>(cube, ColorF4(1.0f, 0.5f, 0.8f), ColorF3(1.0f, 1.0f, 1.0f), 0U, 1.0f, 5.0f, 1.0f);
	Data::Geometry& geo = engine.GetData().emplace<Data::Geometry>(cube, GFX::Resource::VertexBuffer(), GFX::Resource::IndexBuffer());
	Data::MaterialBuffersPBR& mat = engine.GetData().emplace<Data::MaterialBuffersPBR>(cube, Data::MaterialBuffersPBR());

	GFX::Resource::Texture::PackDesc desc;
	std::vector<GFX::Surface> surfaces;
	desc.AddTexture(GFX::Resource::Texture::Type::Tex2D, GFX::Resource::Texture::Usage::PixelShader, std::move(surfaces));
	desc.AddTexture(GFX::Resource::Texture::Type::Tex2D, GFX::Resource::Texture::Usage::PixelShader, std::move(surfaces));
	desc.AddTexture(GFX::Resource::Texture::Type::Tex2D, GFX::Resource::Texture::Usage::PixelShader, std::move(surfaces));
	desc.AddTexture(GFX::Resource::Texture::Type::Tex2D, GFX::Resource::Texture::Usage::PixelShader, std::move(surfaces));
	mat.Init(engine.Gfx().GetDevice(), engine.GetData().get<Data::MaterialPBR>(cube), desc);

	geo.Vertices.Init(engine.Gfx().GetDevice(), { static_cast<U32>(vertices.size()), sizeof(GFX::Vertex), vertices.data() });
	geo.Indices.Init(engine.Gfx().GetDevice(), { static_cast<U32>(indices.size()), indices.data() });

	EID light = engine.GetData().create();
	engine.GetData().emplace<Data::LightPoint>(light);
	engine.GetData().emplace<Data::TransformGlobal>(light, Data::Transform({ 1.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 4.0f, 3.0f }, { 1.0f, 1.0f, 1.0f }));
	Data::PointLight& pointLight = engine.GetData().emplace<Data::PointLight>(light, ColorF3(1.0f, 1.0f, 1.0f), 100.0f, ColorF3(0.05f, 0.05f, 0.05f), 1.0f, 4.0f);
	engine.GetData().emplace<Data::PointLightBuffer>(light,
		Math::GetLightVolume(pointLight.Color, pointLight.Intensity, pointLight.AttnLinear, pointLight.AttnQuad),
		GFX::Resource::CBuffer()).Buffer.Init(engine.Gfx().GetDevice(), &pointLight, sizeof(Data::PointLight), false);

	engine.Gfx().GetDevice().StartUpload();
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