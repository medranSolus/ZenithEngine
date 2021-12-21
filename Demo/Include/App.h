#pragma once
#include "Engine.h"
#include <map>
using namespace ZE;

class App final
{
	static constexpr const char* WINDOW_TITLE = "Zenith Engine v0.3";

	Engine engine;
	bool run = true;

	void ProcessInput();
	void ShowObjectWindow();
	void ShowOptionsWindow();
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