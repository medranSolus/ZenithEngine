#include "App.h"
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

float lightX = 2.0f;
float lightY = 1.0f;
float lightZ = 1.0f;

void App::MakeFrame()
{
	float dTime = timer.Mark();
	angleZ = dTime * rotateZ;
	angleX = dTime * rotateX;
	angleY = dTime * rotateY;
	window.Gfx().BeginFrame(0.1f, 0.1f, 0.1f);
	window.Gfx().SetCamera(camera->GetView());
	pointLight->Bind(window.Gfx());
	while (window.Mouse().IsInput())
	{
		if (auto opt = window.Mouse().Read())
		{
			auto value = opt.value();
			if (value.IsRightDown())
				camera->Rotate(cameraRotateSpeed * static_cast<float>(value.GetDY()) / height, cameraRotateSpeed * static_cast<float>(value.GetDX()) / width);
			switch (value.GetType())
			{
			case WinAPI::Mouse::Event::Type::WheelForward:
			{
				moveZ += 0.001;
				break;
			}
			case WinAPI::Mouse::Event::Type::WheelBackward:
			{
				moveZ -= 0.001;
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
					moveX -= 0.001;
					break;
				}
				case VK_RIGHT:
				{
					moveX += 0.001;
					break;
				}
				case VK_UP:
				{
					moveY += 0.001;
					break;
				}
				case VK_DOWN:
				{
					moveY -= 0.001;
					break;
				}
				case VK_TAB:
				{
					//currScene = ++currScene % 6;
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
	switch (currScene)
	{
	case 0:
	{
		Scene0();
		break;
	}
	case 1:
	{
		Scene1();
		break;
	}
	case 2:
	{
		Scene2();
		break;
	}
	case 3:
	{
		Scene3();
		break;
	}
	case 4:
	{
		Scene4();
		break;
	}
	case 5:
	{
		Scene5();
		break;
	}
	}
	pointLight->SetPos(lightX, lightY, lightZ);
	pointLight->Draw(window.Gfx());
	if (ImGui::Begin("Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::SliderFloat("Rotation X speed", &rotateX, -10.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Rotation Y speed", &rotateY, -10.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Rotation Z speed", &rotateZ, -10.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Light X", &lightX, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Light Y", &lightY, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Light Z", &lightZ, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Camera speed", &cameraSpeed, 0.05f, 0.5f, "%.2f");
		ImGui::SliderFloat("Mouse speed", &cameraRotateSpeed, 1.0f, 5.0f, "%.1f");
		if (ImGui::Button("Reset"))
			moveX = moveY = moveZ = rotateZ = rotateX = rotateY = 0.0f;
	}
	ImGui::End();
	window.Gfx().EndFrame();
}

App::App() : window(width, height, windowTitle)
{
	camera = std::make_unique<Camera>(GetRatio(), 0.5f, 60.0f);
	window.Gfx().Gui().SetFont("Fonts/SparTakus.ttf", 14.0f);
	window.Gfx().SetProjection(camera->GetProjection());
	pointLight = std::make_unique<GFX::Light::PointLight>(window.Gfx(), lightX, lightY, lightZ);
	std::mt19937 engine(std::random_device{}());
	for (unsigned int i = 0; i < 100; ++i)
		boxes.push_back(std::make_unique<GFX::Object::Box>(window.Gfx(), rand(-10.0f, 10.0f, engine), rand(-10.0f, 10.0f, engine), rand(1.0f, 30.0f, engine), rand(6.0f, 15.0f, engine)));
	
	//float width = 1.0f;
	//float height = 1.0f;
	//rect = std::make_unique<GFX::Object::Rectangle>(window.Gfx(), 0.0f, 0.0f, 0.7f, width, height);
	//triangle = std::make_unique<GFX::Object::Triangle>(window.Gfx(), 0.2f, -0.1f, 1.0f, 3.1f, 1.5f, 2.5f);
	//globe = std::make_unique<GFX::Object::Globe>(window.Gfx(), 0.0f, 0.0f, 4.0f, 25, 25, 3.0f, 3.0f);
	//ball = std::make_unique<GFX::Object::Ball>(window.Gfx(), 0.0f, 0.0f, 4.0f, 7, 3.0f);
}

void App::Scene0()
{
	triangle->Update(moveX, moveY, moveZ, angleZ);
	triangle->Draw(window.Gfx());
}

void App::Scene1()
{
	rect->Update(moveX, moveY, moveZ, angleZ);
	rect->Draw(window.Gfx());
}

void App::Scene2()
{
	for (auto & rect : carpetRects)
	{
		rect->Update(moveX, moveY, moveZ, angleZ);
		rect->Draw(window.Gfx());
	}
}

void App::Scene3()
{
	for (auto & b : boxes)
	{
		b->Update(moveX, moveZ, moveY, angleZ, angleX, angleY);
		b->Draw(window.Gfx());
	}
}

void App::Scene4()
{
	globe->Update(moveX, moveY, moveZ, angleZ, angleX, angleY);
	globe->Draw(window.Gfx());
}

void App::Scene5()
{
	ball->Update(moveX, moveY, moveZ, angleZ, angleX, angleY);
	ball->Draw(window.Gfx());
}

unsigned long long App::Run()
{
	//CreateCarpet(4, 0.0f, 0.0f, 10.0f);
	while (true)
	{
		if (const auto status = WinAPI::Window::ProcessMessage())
			return status.value();
		MakeFrame();
	}
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
	for (auto & coord : coordBuffer)
		carpetRects.push_back(std::make_unique<GFX::Object::Rectangle>(window.Gfx(), coord.first, coord.second, 1.0f, width, width, true));
}
