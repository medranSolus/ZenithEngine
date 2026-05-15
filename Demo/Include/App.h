#pragma once
#include "Zenith.h"
#include "Data/Camera.h"
using namespace ZE;

class App final
{
	static constexpr const char* WINDOW_TITLE = "Zenith Engine Demo v" ZE_STRINGIFY(ZE_VERSION_MAJOR) "." ZE_STRINGIFY(ZE_VERSION_MINOR) "." ZE_STRINGIFY(ZE_VERSION_PATCH);
	static constexpr const char* APP_NAME = "Zenith Engine Demo";
	static constexpr float MAX_MOVE_SPEED = 5.0f;

	Engine engine;
	Data::CameraType cameraType = Data::CameraType::Person;
	EID currentCamera = INVALID_EID;
	float moveSpeed = 0.25f;
	float rollSpeed = 0.01f;
	float rotateSpeed = 1.5f;
	bool run = true;
	bool demoWindow = false;

	template<typename T>
	void EnableProperty(EID entity) noexcept;
	template<typename T>
	void DisableProperty(EID entity) noexcept;

	void ProcessInput() noexcept;
	Status ShowOptionsWindow() noexcept;
	void BuiltObjectTree(EID currentEntity, EID& selected) noexcept;
	Status ShowObjectWindow() noexcept;
	void PropagateTransformChange(EID childEntity) noexcept;

	void AddModelButton() noexcept;
	void ChangeBackgroundButton() noexcept;
	void AddLightButton() noexcept;

	EID AddCamera(std::string&& name, float nearZ, float fov,
		Float3&& position, const Float3& angle) noexcept;
	Expected<EID> AddModel(std::string&& name, Float3&& position,
		const Float3& angle, float scale, const std::string& file,
		Data::ExternalModelOptions options = Base(Data::ExternalModelOption::None)) noexcept;
	Expected<EID> AddPointLight(std::string&& name, Float3&& position,
		ColorF3&& color, float intensity, U64 range) noexcept;
	Expected<EID> AddSpotLight(std::string&& name, Float3&& position,
		ColorF3&& color, float intensity, U64 range,
		float innerAngle, float outerAngle, const Float3& direction) noexcept;
	Expected<EID> AddDirectionalLight(std::string&& name,
		ColorF3&& color, float intensity, const Float3& direction) noexcept;

	Status MakeFrame() noexcept;

public:
	App(const CmdParser& params) noexcept
		: engine(SettingsInitParams::GetParsedParams(params, APP_NAME, Settings::ENGINE_VERSION, 0)) {}
	ZE_CLASS_MOVE(App);
	~App() = default;

	Status Init(const CmdParser& params) noexcept;
	Expected<int> Run() noexcept;
};