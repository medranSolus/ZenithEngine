#pragma once
#include "Window.h"
#include "Timer.h"
#include "Shapes.h"
#include "PointLight.h"
#include <map>

class App
{
	static constexpr const char* windowTitle = "Horus Engine Alpha";
	static constexpr float viewDistance = 5000.0f;
	static constexpr float maxMoveSpeed = 3.0f;

	float cameraSpeed = 0.1f;
	float cameraRollSpeed = 0.01;
	float cameraRotateSpeed = 2.0f;

	WinAPI::Window window;
	GFX::Pipeline::RenderCommander renderer;
	Timer timer;
	std::unique_ptr<Camera::ICamera> camera = nullptr;
	std::shared_ptr<GFX::Light::PointLight> pointLight = nullptr;
	std::vector< std::shared_ptr<GFX::IObject>> shapes;
	std::map<std::string, std::shared_ptr<GFX::IObject>> objects;
	std::vector<std::unique_ptr<GFX::Shape::SolidRectangle>> carpetRects;

	inline void ProcessInput();
	inline void ShowObjectWindow();
	inline void ShowOptionsWindow();
	inline void AddShape(std::shared_ptr<GFX::IObject> shape);
	void CreateCarpet(unsigned int depth, float x, float y, float width, GFX::Data::ColorFloat3 color);
	void MakeFrame();

public:
	App(const std::string& commandLine = "");
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	~App() = default;

	unsigned long long Run();
};