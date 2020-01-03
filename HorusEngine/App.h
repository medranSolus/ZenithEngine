#pragma once
#include "Window.h"
#include "Timer.h"
#include "Shapes.h"
#include "Camera.h"
#include "PointLight.h"

class App
{
	static constexpr const char * windowTitle = "Horus Engine Alpha";
	static constexpr float viewDistance = 4194304.0f;

	static unsigned int width;
	static unsigned int height;

	float cameraSpeed = 0.1f;
	float cameraRotateSpeed = 2.0f;
	unsigned int currScene = 3;

	WinAPI::Window window;
	Timer timer;
	std::unique_ptr<Camera> camera = nullptr;
	std::unique_ptr<GFX::Light::PointLight> pointLight = nullptr;
	std::vector<std::unique_ptr<GFX::IObject>> objects;
	std::vector<std::unique_ptr<GFX::Shape::Rectangle>> carpetRects;

	inline void ProcessInput();
	inline void ShowObjectWindow();
	void CreateCarpet(unsigned int depth, float x, float y, float width);
	void MakeFrame();

public:
	App();
	App(const App &) = delete;
	App & operator=(const App &) = delete;
	~App() = default;

	static constexpr float GetRatio() { return static_cast<float>(width) / height; }

	unsigned long long Run();
};
