#include "App.h"
#include "Cameras.h"
#include "Math.h"
#include "ImGui/imgui.h"

unsigned int App::width = 1600;
unsigned int App::height = 900;

float moveX = 0.0f;
float moveY = 0.0f;
float moveZ = 0.0f;

float rotateZ = 0.0f;
float rotateX = 0.0f;
float rotateY = 0.0f;

float angleZ = 0.0f;
float angleX = 0.0f;
float angleY = 0.0f;

inline void App::ProcessInput()
{
	while (window.Mouse().IsInput())
	{
		if (auto opt = window.Mouse().Read())
		{
			auto value = opt.value();
			if (value.IsRightDown() && window.IsCursorEnabled())
				camera->Rotate(cameraRotateSpeed * static_cast<float>(value.GetDY()) / height, cameraRotateSpeed* static_cast<float>(value.GetDX()) / width);
			switch (value.GetType())
			{
			case WinAPI::Mouse::Event::Type::WheelForward:
			{
				moveZ += 0.001;
				if (!window.IsCursorEnabled() && cameraSpeed <= maxMoveSpeed - 0.01f - FLT_EPSILON)
					cameraSpeed += 0.01f;
				break;
			}
			case WinAPI::Mouse::Event::Type::WheelBackward:
			{
				moveZ -= 0.001;
				if (!window.IsCursorEnabled() && cameraSpeed >= 0.012f + FLT_EPSILON)
					cameraSpeed -= 0.01f;
				break;
			}
			case WinAPI::Mouse::Event::Type::RawMove:
			{
				if (!window.IsCursorEnabled())
					camera->Rotate(cameraRotateSpeed * static_cast<float>(value.GetDY()) / height, cameraRotateSpeed* static_cast<float>(value.GetDX()) / width);
				break;
			}
			}
		}
	}
	if (window.Keyboard().IsKeyDown('W'))
		camera->MoveZ(cameraSpeed);
	if (window.Keyboard().IsKeyDown('S'))
		camera->MoveZ(-cameraSpeed);
	if (window.Keyboard().IsKeyDown('A'))
		camera->MoveX(-cameraSpeed);
	if (window.Keyboard().IsKeyDown('D'))
		camera->MoveX(cameraSpeed);
	if (window.Keyboard().IsKeyDown(VK_SPACE))
		camera->MoveY(cameraSpeed);
	if (window.Keyboard().IsKeyDown('C'))
		camera->MoveY(-cameraSpeed);
	if (window.Keyboard().IsKeyDown(VK_LEFT))
		camera->Roll(cameraRollSpeed);
	if (window.Keyboard().IsKeyDown(VK_RIGHT))
		camera->Roll(-cameraRollSpeed);
	while (window.Keyboard().IsKeyReady())
	{
		if (auto opt = window.Keyboard().ReadKey())
		{
			if (opt.value().IsDown())
			{
				switch (opt.value().GetCode())
				{
				case VK_LEFT:
				{
					moveX -= 0.001f;
					break;
				}
				case VK_RIGHT:
				{
					moveX += 0.001f;
					break;
				}
				case VK_UP:
				{
					moveY += 0.001f;
					break;
				}
				case VK_DOWN:
				{
					moveY -= 0.001f;
					break;
				}
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
	if (ImGui::Begin("Object options"))
	{
		static std::map<std::string, std::shared_ptr<GFX::IObject>>::iterator currentItem = objects.find("---None---");
		if (ImGui::BeginCombo("Selected object", currentItem->first.c_str()))
		{
			for (auto it = objects.begin(); it != objects.end(); ++it)
			{
				bool selected = (currentItem == it);
				if (ImGui::Selectable(it->first.c_str(), selected))
					currentItem = it;
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::NewLine();
		if (currentItem->second)
			currentItem->second->ShowWindow();
	}
	ImGui::End();
}

inline void App::ShowOptionsWindow()
{
	if (ImGui::Begin("Options"))
	{
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		//ImGui::SliderFloat("Rotation X speed", &rotateX, -10.0f, 10.0f, "%.2f");
		//ImGui::SliderFloat("Rotation Y speed", &rotateY, -10.0f, 10.0f, "%.2f");
		//ImGui::SliderFloat("Rotation Z speed", &rotateZ, -10.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Move speed", &cameraSpeed, 0.001f, maxMoveSpeed, "%.3f");
		ImGui::SliderFloat("Roll speed", &cameraRollSpeed, 0.01f, 0.5f, "%.2f");
		ImGui::SliderFloat("Camera speed", &cameraRotateSpeed, 1.0f, 5.0f, "%.1f");
		const auto& cameraPos = camera->GetPos();
		ImGui::Text("Camera: [%.3f, %.3f, %.3f]", cameraPos.x, cameraPos.y, cameraPos.z);
		camera->ShowWindow();
		if (ImGui::Button("Reset"))
			moveX = moveY = moveZ = rotateZ = rotateX = rotateY = 0.0f;
	}
	ImGui::End();
}

inline void App::AddShape(std::shared_ptr<GFX::IObject> shape)
{
	shapes.emplace_back(shape);
	objects.emplace(shape->GetName(), shape);
}

void App::CreateCarpet(unsigned int depth, float x, float y, float width)
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
		carpetRects.push_back(std::make_unique<GFX::Shape::Rectangle>(window.Gfx(), DirectX::XMFLOAT3(coord.first, coord.second, 1.0f), "", width, width));
}

void App::MakeFrame()
{
	float dTime = timer.Mark();
	angleZ = dTime * rotateZ;
	angleX = dTime * rotateX;
	angleY = dTime * rotateY;
	window.Gfx().BeginFrame(0.05f, 0.05f, 0.05f);
	ProcessInput();
	camera->Update(window.Gfx());
	pointLight->Draw(window.Gfx());
	pointLight->Bind(window.Gfx(), *camera);
	for (auto& shape : shapes)
		if (shape)
			shape->Draw(window.Gfx());
	for (auto& obj : carpetRects)
	{
		obj->Update({ moveX, moveZ, moveY }, { angleZ, angleX, angleY });
		obj->Draw(window.Gfx());
	}
	ShowObjectWindow();
	ShowOptionsWindow();
	//ImGui::ShowDemoWindow();
	window.Gfx().EndFrame();
}

App::App(const std::string& commandLine) : window(width, height, windowTitle)
{
	objects.emplace("---None---", nullptr);
	camera = std::make_unique<Camera::PersonCamera>(1.047f, GetRatio(), 0.01f, viewDistance);
	window.Gfx().Gui().SetFont("Fonts/Arial.ttf", 14.0f);
	pointLight = std::make_shared<GFX::Light::PointLight>(window.Gfx(), DirectX::XMFLOAT3(0.0f, 0.0f, 4.0f), "PointLight");
	objects.emplace(pointLight->GetName(), pointLight);
	std::mt19937_64 engine(std::random_device{}());
	//AddShape(std::make_shared<GFX::Shape::Box>(window.Gfx(), randPosition(-10.0f, 10.0f, engine), "Box", std::move(randColor(engine)), rand(5.0f, 30.0f, engine)));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/Sting_Sword/Sting_Sword.obj", DirectX::XMFLOAT3(0.0f, -10.0f, 0.0f), "Sting Sword", 4.0f));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/brick_wall/brick_wall.obj", DirectX::XMFLOAT3(0.0f, 4.0f, 10.0f), "Wall"));
	AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/Sponza/sponza.obj", DirectX::XMFLOAT3(0.0f, -8.0f, 0.0f), "Sponza", 0.045f));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/Black Dragon/Dragon 2.5.fbx", DirectX::XMFLOAT3(0.0f, 10.0f, 0.0f), "Black Dragon"));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/nanosuit/nanosuit.obj", DirectX::XMFLOAT3(0.0f, -8.0f, 8.0f), "Nanosuit_tex", 0.78f));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/nano_hierarchy.gltf", DirectX::XMFLOAT3(6.0f, 5.0f, -15.0f), "Nanosuit"));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/boxy.gltf", DirectX::XMFLOAT3(8.0f, 5.0f, -15.0f), "Boxes"));
	//AddShape(std::make_shared<GFX::Shape::Model>(window.Gfx(), "Models/bot/bot.obj", DirectX::XMFLOAT3(-8.0f, -5.0f, -15.0f), "Bot"));
	//AddShape(std::make_unique<GFX::Shape::Rectangle>(window.Gfx(), DirectX::XMFLOAT3(-5.0f, 0.0f, 5.7f), "GetRect", 1.0f, 1.0f));
	//AddShape(std::make_unique<GFX::Shape::Triangle>(window.Gfx(), DirectX::XMFLOAT3(4.2f, -0.1f, 1.0f), "Tri", 3.1f, 1.5f, 2.5f));
	//AddShape(std::make_unique<GFX::Shape::Globe>(window.Gfx(), DirectX::XMFLOAT3(0.0f, 8.0f, -1.0f), "Globe", std::move(randColor(engine)), 25, 25, 2.0f, 3.0f, 1.5f));
	//AddShape(std::make_unique<GFX::Shape::Ball>(window.Gfx(), DirectX::XMFLOAT3(2.0f, 0.0f, -7.0f), "Ball", std::move(randColor(engine)), 3, 3.0f));
}

unsigned long long App::Run()
{
	//CreateCarpet(5, 0.0f, 0.0f, 10.0f);
	while (true)
	{
		if (const auto status = WinAPI::Window::ProcessMessage())
			return status.value();
		MakeFrame();
	}
}