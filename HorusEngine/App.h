#pragma once
#include "Window.h"
#include "Timer.h"
#include "Objects.h"

class App
{
	static constexpr const char * windowTitle = "DirectX 11";

	static unsigned int width;
	static unsigned int height;
	static float screenRatio;

	WinAPI::Window window;
	Timer timer;
	std::unique_ptr<GFX::Object::Rectangle> rect = nullptr;
	std::unique_ptr<GFX::Object::Triangle> triangle = nullptr;
	std::unique_ptr<GFX::Object::Globe> globe = nullptr;
	std::unique_ptr<GFX::Object::Ball> ball = nullptr;
	std::vector<std::unique_ptr<GFX::Object::Box>> boxes;
	void CreateCarpet(unsigned int depth, float x, float y, float width);

	void MakeFrame();

public:
	App();
	App(const App &) = delete;
	App & operator=(const App &) = delete;
	~App() = default;

	void Scene0();
	void Scene1();
	void Scene2();
	void Scene3();
	void Scene4();
	void Scene5();

	static float GetRatio() { return screenRatio; }

	unsigned long long Run();
};
