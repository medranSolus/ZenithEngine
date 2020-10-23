#include "App.h"
#include "Math.h"

#define ContainerInvoke(item, function) \
	switch (item->second.first) \
	{ \
	case Container::Model: \
		models.at(currentItem->second.second).function; \
		break; \
	case Container::Shape: \
		shapes.at(currentItem->second.second)->function; \
		break; \
	case Container::PointLight: \
		pointLights.at(currentItem->second.second).function; \
		break; \
	case Container::SpotLight: \
		spotLights.at(currentItem->second.second).function; \
		break; \
	}

inline void App::ProcessInput()
{
	cameras.ProcessInput(window);
	while (window.Keyboard().IsKeyReady())
	{
		if (auto opt = window.Keyboard().ReadKey())
		{
			if (opt.value().IsDown())
			{
				switch (opt.value().GetCode())
				{
				case VK_ESCAPE:
				{
					window.SwitchCursor();
					break;
				}
				case VK_F1:
				{
					window.Gfx().SwitchGUI();
					break;
				}
				}
			}
		}
	}
}

inline void App::ShowObjectWindow()
{
	static GFX::Probe::ModelProbe probe;
	if (ImGui::Begin("Object options", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
	{
		static std::map<std::string, std::pair<Container, size_t>>::iterator currentItem = objects.find("---None---");
		if (ImGui::BeginCombo("Selected object", currentItem->first.c_str()))
		{
			for (auto it = objects.begin(); it != objects.end(); ++it)
			{
				bool selected = (currentItem == it);
				if (ImGui::Selectable(it->first.c_str(), selected))
				{
					ContainerInvoke(currentItem, DisableOutline());
					currentItem = it;
					probe.ResetNode();
					ContainerInvoke(currentItem, SetOutline());
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::NewLine();
		ContainerInvoke(currentItem, Accept(window.Gfx(), probe));
	}
	ImGui::End();
}

inline void App::ShowOptionsWindow()
{
	static GFX::Probe::BaseProbe probe;
	if (ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
	{
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		cameras.Accept(window.Gfx(), probe);
		ImGui::NewLine();
		renderer.ShowWindow(window.Gfx());
	}
	ImGui::End();
}

inline void App::AddShape(std::shared_ptr<GFX::Shape::IShape> shape)
{
	objects.emplace(shape->GetName(), std::make_pair<Container, size_t>(Container::Shape, shapes.size()));
	shapes.emplace_back(shape);
}

inline void App::AddShape(GFX::Shape::Model&& model)
{
	objects.emplace(model.GetName(), std::make_pair<Container, size_t>(Container::Model, models.size()));
	models.emplace_back(std::forward<GFX::Shape::Model&&>(model));
}

inline void App::AddLight(GFX::Light::PointLight&& pointLight)
{
	objects.emplace(pointLight.GetName(), std::make_pair<Container, size_t>(Container::PointLight, pointLights.size()));
	pointLights.emplace_back(std::forward<GFX::Light::PointLight&&>(pointLight));
}

inline void App::AddLight(GFX::Light::SpotLight&& spotLight)
{
	objects.emplace(spotLight.GetName(), std::make_pair<Container, size_t>(Container::SpotLight, spotLights.size()));
	spotLights.emplace_back(std::forward<GFX::Light::SpotLight&&>(spotLight));
}

void App::CreateCarpet(unsigned int depth, float x, float y, float width, GFX::Data::ColorFloat3 color)
{
	std::deque<std::pair<float, float>> coordBuffer;
	coordBuffer.emplace_back(x, y);
	for (unsigned int d = 0; d < depth; ++d)
	{
		width /= 3.0f;
		for (size_t i = 0, size = coordBuffer.size(); i < size; ++i)
		{
			std::pair<float, float> current = coordBuffer.front();
			coordBuffer.pop_front();
			coordBuffer.emplace_back(current.first - width, current.second - width);
			coordBuffer.emplace_back(current.first - width, current.second);
			coordBuffer.emplace_back(current.first - width, current.second + width);
			coordBuffer.emplace_back(current.first, current.second + width);
			coordBuffer.emplace_back(current.first + width, current.second + width);
			coordBuffer.emplace_back(current.first + width, current.second);
			coordBuffer.emplace_back(current.first + width, current.second - width);
			coordBuffer.emplace_back(current.first, current.second - width);
		}
	}
	for (auto& coord : coordBuffer)
		carpetRects.push_back(std::make_unique<GFX::Shape::SolidRectangle>(window.Gfx(), renderer,
			DirectX::XMFLOAT3(coord.first, coord.second, 1.0f), "", color, width, width));
}

void App::MakeFrame()
{
	window.Gfx().BeginFrame();
	ProcessInput();
	ShowObjectWindow();
	ShowOptionsWindow();
	//ImGui::ShowDemoWindow();
	if (cameras.CameraChanged())
		renderer.BindMainCamera(cameras.GetCamera());
	cameras.Submit(RenderChannel::Main);
	for (auto& shape : shapes)
		shape->Submit(RenderChannel::Main | RenderChannel::Shadow);
	for (auto& model : models)
		model.Submit(RenderChannel::Main | RenderChannel::Shadow);
	for (auto& pointLight : pointLights)
		pointLight.Submit(RenderChannel::Main | RenderChannel::Light);
	for (auto& spotLight : spotLights)
		spotLight.Submit(RenderChannel::Main | RenderChannel::Light);
	renderer.Execute(window.Gfx());
	renderer.Reset();
	window.Gfx().EndFrame();
}

App::App(const std::string& commandLine)
	: window(1600, 900, WINDOW_TITLE), renderer(window.Gfx()),
	cameras(std::make_unique<Camera::PersonCamera>(window.Gfx(), renderer, "Camera_1", 1.047f, 0.01f, VIEW_DISTANCE, 90, 0, DirectX::XMFLOAT3(-8.0f, 0.0f, 0.0f)))
{
	window.Gfx().Gui().SetFont("Fonts/Arial.ttf", 14.0f);
	objects.emplace("---None---", std::make_pair<Container, size_t>(Container::None, 0));
	AddLight({ window.Gfx(), renderer, DirectX::XMFLOAT3(-20.0f, 1.0f, -4.0f), "Light bulb", 50 });
	AddLight({ window.Gfx(), renderer, DirectX::XMFLOAT3(14.0f, -6.3f, -5.0f), "Pumpkin candle", 85, 5.0f, GFX::Data::ColorFloat3(1.0f, 0.96f, 0.27f) });
	AddLight({ window.Gfx(), renderer, DirectX::XMFLOAT3(21.95f, -1.9f, 9.9f), "Torch", 70, 5.0f, GFX::Data::ColorFloat3(1.0f, 0.0f, 0.2f) });
	AddLight({ window.Gfx(), renderer, DirectX::XMFLOAT3(43.0f, 27.0f, 1.8f), "Blue ilumination", 70, 10.0f, GFX::Data::ColorFloat3(0.0f, 0.46f, 1.0f) });
	AddLight({ window.Gfx(), renderer, DirectX::XMFLOAT3(7.5f, 0.0f, -5.0f), "Flashlight", 50, 20.0f, 40.0f,
		DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), 2.0f, GFX::Data::ColorFloat3(0.5f, 1.0f, 0.5f) });
	AddLight({ window.Gfx(), renderer, DirectX::XMFLOAT3(7.5f, 0.0f, 5.0f), "Cold light", 80, 5.0f, 20.0f,
		DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), 9.0f, GFX::Data::ColorFloat3(0.5f, 0.5f, 1.0f) });
	cameras.AddCamera(std::make_unique<Camera::PersonCamera>(window.Gfx(), renderer, "Camera_2",
		1.047f, 0.01f, VIEW_DISTANCE, 0, 90, DirectX::XMFLOAT3(0.0f, 8.0f, -8.0f)));
	AddShape({ window.Gfx(), renderer, "Models/Sponza/sponza.obj", DirectX::XMFLOAT3(0.0f, -8.0f, 0.0f), "Sponza", 0.045f });
	AddShape({ window.Gfx(), renderer, "Models/nanosuit/nanosuit.obj", DirectX::XMFLOAT3(0.0f, -8.2f, 6.0f), "Nanosuit", 0.70f });
	AddShape({ window.Gfx(), renderer, "Models/Jack/Jack_O_Lantern.3ds", DirectX::XMFLOAT3(13.5f, -8.2f, -5.0f), "Jack O'Lantern", 13.00f });
	AddShape({ window.Gfx(), renderer, "Models/bricks/brick_wall.obj", DirectX::XMFLOAT3(-5.0f, -2.0f, 7.0f), "Wall", 2.0f });
	//AddShape({ window.Gfx(), renderer, "Models/Black Dragon/Dragon 2.5.fbx", DirectX::XMFLOAT3(0.0f, 10.0f, 0.0f), "Black Dragon", 0.15f });
	//std::mt19937_64 engine(std::random_device{}());
	//AddShape(std::make_shared<GFX::Shape::Box>(window.Gfx(), RandPosition(-10.0f, 10.0f, engine), "Box", std::move(RandColor(engine)), Rand(5.0f, 30.0f, engine)));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), renderer, "Models/Sting_Sword/Sting_Sword.obj", DirectX::XMFLOAT3(0.0f, -2.0f, 3.0f), "Sting Sword", 0.5f));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/brick_wall/brick_wall.obj", DirectX::XMFLOAT3(0.0f, 4.0f, 10.0f), "Wall"));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/nano_hierarchy.gltf", DirectX::XMFLOAT3(6.0f, 5.0f, -15.0f), "Nanosuit_old"));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/boxy.gltf", DirectX::XMFLOAT3(8.0f, 5.0f, -15.0f), "Boxes"));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/bot/bot.obj", DirectX::XMFLOAT3(-8.0f, -5.0f, -15.0f), "Bot"));
	//AddShape(std::make_unique<GFX::Shape::SolidRectangle>(window.Gfx(), DirectX::XMFLOAT3(-5.0f, 0.0f, 5.7f), "GetRect", 1.0f, 1.0f));
	//AddShape(std::make_unique<GFX::Shape::Triangle>(window.Gfx(), DirectX::XMFLOAT3(4.2f, -0.1f, 1.0f), "Tri", 3.1f, 1.5f, 2.5f));
	//AddShape(std::make_unique<GFX::Shape::Globe>(window.Gfx(), DirectX::XMFLOAT3(0.0f, -2.0f, 8.0f), "Globe", std::move(RandColor(engine)), 25, 25, 3.0f, 3.0f, 3.0f));
	//AddShape(std::make_shared<GFX::Shape::Ball>(window.Gfx(), renderer, DirectX::XMFLOAT3(0.0f, -3.0f, 0.0f), "Ball", std::move(Math::RandColor(engine)), 3, 1.0f));
}

size_t App::Run()
{
	while (true)
	{
		if (const auto status = WinAPI::Window::ProcessMessage())
			return status.value();
		MakeFrame();
	}
}