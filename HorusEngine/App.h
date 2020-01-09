#pragma once
#include "Window.h"
#include "Timer.h"
#include "Shapes.h"
#include "Camera.h"
#include "PointLight.h"
#include <map>

class App
{
	static constexpr const char * windowTitle = "Horus Engine Alpha";
	static constexpr float viewDistance = 4194304.0f;

	static unsigned int width;
	static unsigned int height;

	float cameraSpeed = 0.1f;
	float cameraRollSpeed = 0.01;
	float cameraRotateSpeed = 2.0f;

	WinAPI::Window window;
	Timer timer;
	std::unique_ptr<Camera> camera = nullptr;
	std::shared_ptr<GFX::Light::PointLight> pointLight = nullptr;
	std::vector< std::shared_ptr<GFX::IObject>> shapes;
	std::map<std::string, std::shared_ptr<GFX::IObject>> objects;
	std::vector<std::unique_ptr<GFX::Shape::Rectangle>> carpetRects;
	
	inline void ProcessInput();
	inline void ShowObjectWindow();
	inline void ShowOptionsWindow();
	inline void AddShape(std::shared_ptr<GFX::IObject> shape);
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
