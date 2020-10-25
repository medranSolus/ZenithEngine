#pragma once
#include "Timer.h"
#include "Cameras.h"
#include "Shapes.h"
#include "Lights.h"
#include "MainPipelineGraph.h"
#include <map>

class App
{
	enum class Container { None, PointLight, SpotLight, DirectionalLight, Model, Shape };

	static constexpr const char* WINDOW_TITLE = "Horus Engine Alpha";
	static constexpr float VIEW_DISTANCE = 500.0f;

	WinAPI::Window window;
	GFX::Pipeline::MainPipelineGraph renderer;
	Camera::CameraPool cameras;
	Timer timer;

	std::vector<GFX::Light::PointLight> pointLights;
	std::vector<GFX::Light::SpotLight> spotLights;
	std::vector<GFX::Light::DirectionalLight> directionalLights;
	std::vector<GFX::Shape::Model> models;
	std::vector<std::shared_ptr<GFX::Shape::IShape>> shapes;

	std::map<std::string, std::pair<Container, size_t>> objects;
	std::vector<std::unique_ptr<GFX::Shape::SolidRectangle>> carpetRects;

	inline void AddLight(GFX::Light::PointLight&& pointLight);
	inline void AddLight(GFX::Light::SpotLight&& spotLight);
	inline void AddLight(GFX::Light::DirectionalLight&& directionalLight);
	inline void AddShape(GFX::Shape::Model&& model);
	inline void AddShape(std::shared_ptr<GFX::Shape::IShape> shape);

	inline void ProcessInput();
	inline void ShowObjectWindow();
	inline void ShowOptionsWindow();

	void CreateCarpet(unsigned int depth, float x, float y, float width, GFX::Data::ColorFloat3 color);
	void MakeFrame();

public:
	App(const std::string& commandLine = "");
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	~App() = default;

	size_t Run();
};