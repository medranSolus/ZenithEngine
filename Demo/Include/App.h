#pragma once
#include "Engine.h"
#include <map>
using namespace ZE;

class App final
{
	static constexpr const char* WINDOW_TITLE = "Zenith Engine v0.3";
	static constexpr float MAX_MOVE_SPEED = 5.0f;

	Engine engine;
	Data::CameraType cameraType = Data::CameraType::Person;
	EID camera;
	EID cube;
	float moveSpeed = 0.02f;
	float rollSpeed = 0.01f;
	float rotateSpeed = 1.5f;
	Matrix currentProjection;
	bool run = true;

	void ProcessInput();
	void ShowOptionsWindow();

	void ShowObjectWindow();
	void AddModelButton();
	void ChangeBackgroundButton();
	void AddLightButton();
	void MakeFrame();

public:
	App(const std::string& commandLine = "");
	ZE_CLASS_DELETE(App);
	~App() = default;

	int Run();
};