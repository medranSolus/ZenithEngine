#pragma once
#include "Zenith.h"
using namespace ZE;

class App final
{
	static constexpr const char* WINDOW_TITLE = "Zenith Engine v0.3";
	static constexpr const char* APP_NAME = "Zenith Engine Demo";
	static constexpr float MAX_MOVE_SPEED = 5.0f;

	Engine engine;
	Data::CameraType cameraType = Data::CameraType::Person;
	EID currentCamera;
	float moveSpeed = 0.25f;
	float rollSpeed = 0.01f;
	float rotateSpeed = 1.5f;
	bool run = true;

	template<typename T>
	void EnableProperty(EID entity);
	template<typename T>
	void DisableProperty(EID entity);

	void ProcessInput();
	void ShowOptionsWindow();
	void BuiltObjectTree(EID currentEntity, EID& selected);
	void ShowObjectWindow();

	void AddModelButton();
	void ChangeBackgroundButton();
	void AddLightButton();

	EID AddCamera(std::string&& name, float nearZ, float fov,
		Float3&& position, const Float3& angle);
	EID AddModel(std::string&& name, Float3&& position,
		const Float3& angle, float scale, const std::string& file);
	EID AddPointLight(std::string&& name, Float3&& position,
		ColorF3&& color, float intensity, U64 range);
	EID AddSpotLight(std::string&& name, Float3&& position,
		ColorF3&& color, float intensity, U64 range,
		float innerAngle, float outerAngle, const Float3& direction);
	EID AddDirectionalLight(std::string&& name,
		ColorF3&& color, float intensity, const Float3& direction);
	void MakeFrame();

public:
	App(const CmdParser& params);
	ZE_CLASS_DELETE(App);
	~App() = default;

	int Run();
};