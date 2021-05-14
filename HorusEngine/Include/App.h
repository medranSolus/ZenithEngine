#pragma once
#include "Timer.h"
#include "Camera/Cameras.h"
#include "GFX/Shape/Shapes.h"
#include "GFX/Light/Lights.h"
#include "GFX/Pipeline/MainPipelineGraph.h"
#include <map>

class App final
{
	enum class Container : U8 { None, PointLight, SpotLight, DirectionalLight, Model, Shape };

	static constexpr const char* WINDOW_TITLE = "Zenith Engine v0.2";

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

	void AddLight(GFX::Light::PointLight&& pointLight);
	void AddLight(GFX::Light::SpotLight&& spotLight);
	void AddLight(GFX::Light::DirectionalLight&& directionalLight);
	void AddShape(GFX::Shape::Model&& model);
	void AddShape(std::shared_ptr<GFX::Shape::IShape> shape);
	void DeleteObject(std::map<std::string, std::pair<Container, U64>>::iterator& object) noexcept;

	void ProcessInput();
	void ShowObjectWindow();
	void ShowOptionsWindow();
	void AddModelButton();
	void ChangeBackgroundButton();
	void AddLightButton();
	void MakeFrame();

public:
	App(const std::string& commandLine = "");
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	~App() = default;

	int Run();
};