#include "App.h"
#include "Math.h"

inline void App::ProcessInput()
{
	cameras->ProcessInput(window);
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
	if (ImGui::Begin("Object options"))
	{
		static std::map<std::string, std::shared_ptr<GFX::IObject>>::iterator currentItem = objects.find("---None---");
		if (ImGui::BeginCombo("Selected object", currentItem->first.c_str()))
		{
			for (auto it = objects.begin(); it != objects.end(); ++it)
			{
				bool selected = (currentItem == it);
				if (ImGui::Selectable(it->first.c_str(), selected))
				{
					if (currentItem->second)
						currentItem->second->DisableOutline();
					currentItem = it;
					probe.ResetNode();
					if (currentItem->second)
						currentItem->second->SetOutline();
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::NewLine();
		if (currentItem->second)
			currentItem->second->Accept(window.Gfx(), probe);
	}
	ImGui::End();
}

inline void App::ShowOptionsWindow()
{
	static GFX::Probe::BaseProbe probe;
	if (ImGui::Begin("Options"))
	{
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		cameras->Accept(window.Gfx(), probe);
		ImGui::NewLine();
		renderer.ShowWindow();
	}
	ImGui::End();
}

inline void App::AddShape(std::shared_ptr<GFX::IObject> shape)
{
	shapes.emplace_back(shape);
	objects.emplace(shape->GetName(), shape);
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
		carpetRects.push_back(std::make_unique<GFX::Shape::SolidRectangle>(window.Gfx(), renderer, DirectX::XMFLOAT3(coord.first, coord.second, 1.0f), "", color, width, width));
}

void App::MakeFrame()
{
	window.Gfx().BeginFrame();
	ProcessInput();
	renderer.BindMainCamera(cameras->GetCamera());
	cameras->Submit(RenderChannel::Main);
	pointLight->Submit(RenderChannel::Main);
	pointLight->Bind(window.Gfx(), cameras->GetCamera());
	for (auto& shape : shapes)
		if (shape)
			shape->Submit(RenderChannel::Main | RenderChannel::Shadow);
	for (auto& obj : carpetRects)
		obj->Submit(RenderChannel::Main);
	ShowObjectWindow();
	ShowOptionsWindow();
	//ImGui::ShowDemoWindow();
	renderer.Execute(window.Gfx());
	renderer.Reset();
	window.Gfx().EndFrame();
}

App::App(const std::string& commandLine) : window(1600, 900, windowTitle), renderer(window.Gfx())
{
	objects.emplace("---None---", nullptr);
	cameras = std::make_unique<Camera::CameraPool>(std::make_unique<Camera::PersonCamera>(window.Gfx(), renderer, "Camera_1",
		1.047f, 0.01f, viewDistance, 90, 0, DirectX::XMFLOAT3(-8.0f, 0.0f, 0.0f)));
	cameras->AddCamera(std::make_unique<Camera::PersonCamera>(window.Gfx(), renderer, "Camera_2",
		1.047f, 0.01f, viewDistance, 0, 90, DirectX::XMFLOAT3(0.0f, 8.0f, -8.0f)));
	window.Gfx().Gui().SetFont("Fonts/Arial.ttf", 14.0f);
	pointLight = std::make_shared<GFX::Light::PointLight>(window.Gfx(), renderer, DirectX::XMFLOAT3(0.0f, 1.0f, -4.0f), "PointLight");
	objects.emplace(pointLight->GetName(), pointLight);
	std::mt19937_64 engine(std::random_device{}());
	AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), renderer, "Models/Sponza/sponza.obj", DirectX::XMFLOAT3(0.0f, -8.0f, 0.0f), "Sponza", 0.045f));
	AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), renderer, "Models/nanosuit/nanosuit.obj", DirectX::XMFLOAT3(0.0f, -8.0f, 6.0f), "Nanosuit", 0.70f));
	//AddShape(std::make_shared<GFX::Shape::Box>(window.Gfx(), RandPosition(-10.0f, 10.0f, engine), "Box", std::move(RandColor(engine)), Rand(5.0f, 30.0f, engine)));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/Sting_Sword/Sting_Sword.obj", DirectX::XMFLOAT3(0.0f, -10.0f, 0.0f), "Sting Sword", 4.0f));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/brick_wall/brick_wall.obj", DirectX::XMFLOAT3(0.0f, 4.0f, 10.0f), "Wall"));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/Black Dragon/Dragon 2.5.fbx", DirectX::XMFLOAT3(0.0f, 10.0f, 0.0f), "Black Dragon"));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/nano_hierarchy.gltf", DirectX::XMFLOAT3(6.0f, 5.0f, -15.0f), "Nanosuit_old"));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/boxy.gltf", DirectX::XMFLOAT3(8.0f, 5.0f, -15.0f), "Boxes"));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/bot/bot.obj", DirectX::XMFLOAT3(-8.0f, -5.0f, -15.0f), "Bot"));
	//AddShape(std::make_unique<GFX::Shape::SolidRectangle>(window.Gfx(), DirectX::XMFLOAT3(-5.0f, 0.0f, 5.7f), "GetRect", 1.0f, 1.0f));
	//AddShape(std::make_unique<GFX::Shape::Triangle>(window.Gfx(), DirectX::XMFLOAT3(4.2f, -0.1f, 1.0f), "Tri", 3.1f, 1.5f, 2.5f));
	//AddShape(std::make_unique<GFX::Shape::Globe>(window.Gfx(), DirectX::XMFLOAT3(0.0f, -2.0f, 8.0f), "Globe", std::move(RandColor(engine)), 25, 25, 3.0f, 3.0f, 3.0f));
	//AddShape(std::make_unique<GFX::Shape::Ball>(window.Gfx(), DirectX::XMFLOAT3(2.0f, 0.0f, -7.0f), "Ball", std::move(RandColor(engine)), 3, 3.0f));
	renderer.BindLight(*pointLight);
}

unsigned long long App::Run()
{
	while (true)
	{
		if (const auto status = WinAPI::Window::ProcessMessage())
			return status.value();
		MakeFrame();
	}
}