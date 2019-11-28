#include "App.h"
#include "Math.h"
#include "ImGui/imgui.h"

unsigned int App::width = 1366;
unsigned int App::height = 768;
float App::screenRatio = static_cast<float>(App::height) / App::width;

// Test code
struct Carpet
{
	std::vector<std::unique_ptr<GFX::Object::Rectangle>> rects;
} carpet;
constexpr float depth = 40.0f;
float dTime = 0.0f;
float moveX = 0.0f;
float moveY = 0.0f;
float moveZ = 0.0f;
float rotateZ = 0.0f;
float rotateX = 0.0f;
float rotateY = 0.0f;
float angleZ = 0.0f;
float angleX = 0.0f;
float angleY = 0.0f;
unsigned int currScene = 1;

void App::MakeFrame()
{
	dTime = timer.Mark();
	angleZ = dTime * rotateZ;
	angleX = dTime * rotateX;
	angleY = dTime * rotateY;
	window.SetTitle("Time " + std::to_string(angleZ));
	window.Gfx().BeginFrame(0.2f, 0.2f, 0.2f);
	/*if (window.Mouse().IsLeftDown())
	{
		moveX = static_cast<float>(window.Mouse().GetX()) / (App::GetWidth() / 2) - 1.0f;
		moveY = static_cast<float>(window.Mouse().GetY()) / (-App::GetHeight() / 2) + 1.0f;
	}*/
	while (window.Mouse().IsInput())
	{
		if (auto opt = window.Mouse().Read())
		{
			switch (opt.value().GetType())
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
	while (window.Keyboard().IsKeyReady())
	{
		if (auto opt = window.Keyboard().ReadKey())
		{
			if (opt.value().IsDown())
			{
				switch (opt.value().GetCode())
				{
				case 'W':
				{
					moveY -= 0.001f;
					break;
				}
				case 'S':
				{
					moveY += 0.001f;
					break;
				}
				case 'A':
				{
					moveX += 0.001f;
					break;
				}
				case 'D':
				{
					moveX -= 0.001f;
					break;
				}
				case VK_TAB:
				{
					currScene = ++currScene % 6;
					break;
				}
				case VK_SPACE:
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
	case 6:
	{
		Scene0();
		Scene1();
		Scene2();
		Scene3();
		Scene4();
		Scene5();
		break;
	}
	}
	if (ImGui::Begin("Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::SliderFloat("Rotation X speed", &rotateX, -10.0f, 10.0f);
		ImGui::SliderFloat("Rotation Y speed", &rotateY, -10.0f, 10.0f);
		ImGui::SliderFloat("Rotation Z speed", &rotateZ, -10.0f, 10.0f);
		if (ImGui::Button("Reset"))
			moveX = moveY = moveZ = rotateZ = rotateX = rotateY = 0.0f;
	}
	ImGui::End();
	window.Gfx().EndFrame();
}

App::App() : window(width, height, windowTitle)
{
	window.Gfx().Gui().SetFont("Fonts/SparTakus.ttf", 14.0f);
	window.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, screenRatio, 0.5f, 40.0f));
	window.Gfx().SetCamera(DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.5f));

	std::mt19937 engine(std::random_device{}());
	for (unsigned int i = 0; i < 100; ++i)
		boxes.push_back(std::make_unique<GFX::Object::Box>(window.Gfx(), randWrapNDC(), randWrapNDC(), randWrapNDC(), rand(6.0f, 20.0f, engine)));
	float width = 1.0f;
	float height = 1.0f;
	rect = std::make_unique<GFX::Object::Rectangle>(window.Gfx(), 0.0f, 0.0f, 0.7f, width, height);
	triangle = std::make_unique<GFX::Object::Triangle>(window.Gfx(), 0.2f, -0.1f, 1.0f, 3.1f, 1.5f, 2.5f);
	globe = std::make_unique<GFX::Object::Globe>(window.Gfx(), 0.0f, 0.0f, 4.0f, 25, 25, 3.0f, 3.0f);
	ball = std::make_unique<GFX::Object::Ball>(window.Gfx(), 0.0f, 0.0f, 4.0f, 4, 3.0f);
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
	for (auto & rect : carpet.rects)
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
	CreateCarpet(4, moveX, moveY, 10.0f);
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
		carpet.rects.push_back(std::make_unique<GFX::Object::Rectangle>(window.Gfx(), coord.first, coord.second, 1.0f, width, width, true));
}
