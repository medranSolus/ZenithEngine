#pragma once
#include "Window.h"
#include "Timer.h"
#include "Objects.h"
#include "Camera.h"
#include "PointLight.h"

class App
{
	static constexpr const char * windowTitle = "Horus Engine Alpha";

	static unsigned int width;
	static unsigned int height;

	float cameraSpeed = 0.1f;
	float cameraRotateSpeed = 2.0f;
	unsigned int currScene = 3;

	WinAPI::Window window;
	Timer timer;
	std::unique_ptr<Camera> camera = nullptr;
	std::unique_ptr<GFX::Light::PointLight> pointLight = nullptr;
	std::unique_ptr<GFX::Object::Rectangle> rect = nullptr;
	std::unique_ptr<GFX::Object::Triangle> triangle = nullptr;
	std::unique_ptr<GFX::Object::Globe> globe = nullptr;
	std::unique_ptr<GFX::Object::Ball> ball = nullptr;
	std::vector<std::unique_ptr<GFX::Object::Box>> boxes;
	std::vector<std::unique_ptr<GFX::Object::Rectangle>> carpetRects;
	void CreateCarpet(unsigned int depth, float x, float y, float width);

	void MakeFrame();

public:
	App();
	App(const App &) = delete;
	App & operator=(const App &) = delete;
	~App() = default;

	void Scene0();
	void Scene1();
	void Scene2();
	void Scene3();
	void Scene4();
	void Scene5();

	static constexpr float GetRatio() { return static_cast<float>(width) / height; }

	unsigned long long Run();
};
