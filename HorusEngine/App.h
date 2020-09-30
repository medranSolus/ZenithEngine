#pragma once
#include "Timer.h"
#include "Cameras.h"
#include "Shapes.h"
#include "PointLight.h"
#include "RenderGraphBlurOutline.h"
#include <map>

class App
{
	static constexpr const char* WINDOW_TITLE = "Horus Engine Alpha";
	static constexpr float VIEW_DISTANCE = 500.0f;

	WinAPI::Window window;
	GFX::Pipeline::RenderGraphBlurOutline renderer;
	Camera::CameraPool cameras;
	Timer timer;
	GFX::Light::PointLight pointLight;
	std::vector<GFX::Shape::Model> models;
	std::vector<std::shared_ptr<GFX::Shape::IShape>> shapes;
	std::map<std::string, GFX::IObject*> objects;
	std::vector<std::unique_ptr<GFX::Shape::SolidRectangle>> carpetRects;

	inline void ProcessInput();
	inline void ShowObjectWindow();
	inline void ShowOptionsWindow();
	inline void AddShape(std::shared_ptr<GFX::Shape::IShape> shape);
	inline void AddShape(GFX::Shape::Model&& model);
	void CreateCarpet(unsigned int depth, float x, float y, float width, GFX::Data::ColorFloat3 color);
	void MakeFrame();

public:
	App(const std::string& commandLine = "");
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	~App() = default;

	size_t Run();
};