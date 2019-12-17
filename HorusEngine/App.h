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
	std::vector<std::unique_ptr<GFX::Object::IDrawable>> objects;

	std::vector<std::unique_ptr<GFX::Object::Box>> boxes;
	std::vector<std::unique_ptr<GFX::Object::Rectangle>> carpetRects;
	void CreateCarpet(unsigned int depth, float x, float y, float width);

	void MakeFrame();
	inline void ProcessInput();

public:
	App();
	App(const App &) = delete;
	App & operator=(const App &) = delete;
	~App() = default;

	static constexpr float GetRatio() { return static_cast<float>(width) / height; }

	unsigned long long Run();
};
