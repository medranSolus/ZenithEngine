#pragma once
#include "Timer.h"
#include "Cameras.h"
#include "Shapes.h"
#include "Lights.h"
#include "MainPipelineGraph.h"
#include <map>

class App
{
	enum class Container : uint8_t { None, PointLight, SpotLight, DirectionalLight, Model, Shape };

	static constexpr const char* WINDOW_TITLE = "Horus Engine Alpha";

	WinAPI::Window window;
	GFX::Pipeline::MainPipelineGraph renderer;
	Camera::CameraPool cameras;
	bool run = true;

	std::vector<GFX::Light::PointLight> pointLights;
	std::vector<GFX::Light::SpotLight> spotLights;
	std::vector<GFX::Light::DirectionalLight> directionalLights;
	std::vector<GFX::Shape::Model> models;
	std::vector<std::shared_ptr<GFX::Shape::IShape>> shapes;
	std::map<std::string, std::pair<Container, size_t>> objects;

	inline void AddLight(GFX::Light::PointLight&& pointLight);
	inline void AddLight(GFX::Light::SpotLight&& spotLight);
	inline void AddLight(GFX::Light::DirectionalLight&& directionalLight);
	inline void AddShape(GFX::Shape::Model&& model);
	inline void AddShape(std::shared_ptr<GFX::Shape::IShape> shape);
	inline void DeleteObject(std::map<std::string, std::pair<Container, size_t>>::iterator& object) noexcept;

	inline void ProcessInput();
	inline void ShowObjectWindow();
	inline void ShowOptionsWindow();
	inline void AddModelButton();
	inline void ChangeBackgroundButton();
	inline void AddLightButton();
	inline void MakeFrame();
	//void CreateCarpet(unsigned int depth, float x, float y, float width, GFX::Data::ColorFloat3 color);

public:
	App(const std::string& commandLine = "");
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	~App() = default;

	size_t Run();
};