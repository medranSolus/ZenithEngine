#pragma once
#include "Timer.h"
#include "Cameras.h"
#include "Shapes.h"
#include "PointLight.h"
#include "MainPipelineGraph.h"
#include <map>

class App
{
	enum class Container { None, Model, Shape, PointLight };

	static constexpr const char* WINDOW_TITLE = "Horus Engine Alpha";
	static constexpr float VIEW_DISTANCE = 500.0f;

	WinAPI::Window window;
	GFX::Pipeline::MainPipelineGraph renderer;
	Camera::CameraPool cameras;
	Timer timer;
	std::vector<GFX::Shape::Model> models;
	std::vector<std::shared_ptr<GFX::Shape::IShape>> shapes;
	std::vector<GFX::Light::PointLight> pointLights;
	std::map<std::string, std::pair<Container, size_t>> objects;
	std::vector<std::unique_ptr<GFX::Shape::SolidRectangle>> carpetRects;

	inline void ProcessInput();
	inline void ShowObjectWindow();
	inline void ShowOptionsWindow();
	inline void AddShape(std::shared_ptr<GFX::Shape::IShape> shape);
	inline void AddShape(GFX::Shape::Model&& model);
	inline void AddLight(GFX::Light::PointLight&& pointLight);
	void CreateCarpet(unsigned int depth, float x, float y, float width, GFX::Data::ColorFloat3 color);
	void MakeFrame();

public:
	App(const std::string& commandLine = "");
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	~App() = default;

	size_t Run();
};