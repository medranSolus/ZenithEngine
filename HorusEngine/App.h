#pragma once
#include "Timer.h"
#include "Cameras.h"
#include "Shapes.h"
#include "PointLight.h"
#include "RenderGraphBlurOutline.h"
#include <map>

class App
{
	static constexpr const char* windowTitle = "Horus Engine Alpha";
	static constexpr float viewDistance = 500.0f;

	WinAPI::Window window;
	GFX::Pipeline::RenderGraphBlurOutline renderer;
	Timer timer;
	std::unique_ptr<Camera::CameraPool> cameras = nullptr;
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