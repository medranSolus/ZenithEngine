#include "App.h"
#include "GUI/DialogWindow.h"

#pragma region Containers methods
#define ContainerInvoke(item, function) \
	switch (item->second.first) \
	{ \
	case Container::PointLight: \
		pointLights.at(item->second.second).function; \
		break; \
	case Container::SpotLight: \
		spotLights.at(item->second.second).function; \
		break; \
	case Container::DirectionalLight: \
		directionalLights.at(item->second.second).function; \
		break; \
	case Container::Model: \
		models.at(item->second.second).function; \
		break; \
	case Container::Shape: \
		shapes.at(item->second.second)->function; \
		break; \
	}

void App::AddLight(GFX::Light::PointLight&& pointLight)
{
	objects.emplace(pointLight.GetName(), std::make_pair(Container::PointLight, pointLights.size()));
	pointLights.emplace_back(std::forward<GFX::Light::PointLight>(pointLight));
}

void App::AddLight(GFX::Light::SpotLight&& spotLight)
{
	objects.emplace(spotLight.GetName(), std::make_pair(Container::SpotLight, spotLights.size()));
	spotLights.emplace_back(std::forward<GFX::Light::SpotLight>(spotLight));
}

void App::AddLight(GFX::Light::DirectionalLight&& directionalLight)
{
	objects.emplace(directionalLight.GetName(), std::make_pair(Container::DirectionalLight, directionalLights.size()));
	directionalLights.emplace_back(std::forward<GFX::Light::DirectionalLight>(directionalLight));
}

void App::AddShape(GFX::Shape::Model&& model)
{
	objects.emplace(model.GetName(), std::make_pair(Container::Model, models.size()));
	models.emplace_back(std::forward<GFX::Shape::Model>(model));
}

void App::AddShape(std::shared_ptr<GFX::Shape::IShape> shape)
{
	objects.emplace(shape->GetName(), std::make_pair(Container::Shape, shapes.size()));
	shapes.emplace_back(std::move(shape));
}

void App::DeleteObject(std::map<std::string, std::pair<Container, U64>>::iterator& object) noexcept
{
	switch (object->second.first)
	{
	case Container::PointLight:
	{
		if (pointLights.size() > 1)
		{
			objects.at(pointLights.back().GetName()) = object->second;
			pointLights.at(object->second.second) = std::move(pointLights.back());
		}
		pointLights.pop_back();
		break;
	}
	case Container::SpotLight:
	{
		if (spotLights.size() > 1)
		{
			objects.at(spotLights.back().GetName()) = object->second;
			spotLights.at(object->second.second) = std::move(spotLights.back());
		}
		spotLights.pop_back();
		break;
	}
	case Container::DirectionalLight:
	{
		if (directionalLights.size() > 1)
		{
			objects.at(directionalLights.back().GetName()) = object->second;
			directionalLights.at(object->second.second) = std::move(directionalLights.back());
		}
		directionalLights.pop_back();
		break;
	}
	case Container::Model:
	{
		if (models.size() > 1)
		{
			objects.at(models.back().GetName()) = object->second;
			models.at(object->second.second) = std::move(models.back());
		}
		models.pop_back();
		break;
	}
	case Container::Shape:
	{
		if (shapes.size() > 1)
		{
			objects.at(shapes.back()->GetName()) = object->second;
			shapes.at(object->second.second) = std::move(shapes.back());
		}
		shapes.pop_back();
		break;
	}
	}
	if (object->second.first != Container::None)
	{
		objects.erase(object);
		object = objects.find("---None---");
	}
}
#pragma endregion

void App::ProcessInput()
{
	cameras.ProcessInput(window);
	while (window.Keyboard().IsKeyReady())
	{
		if (auto opt = window.Keyboard().ReadKey())
		{
			if (opt.value().IsDown())
			{
				switch (opt.value().GetCode())
				{
				case VK_ESCAPE:
				{
					window.SwitchCursor();
					break;
				}
				case VK_F1:
				{
					window.Gfx().SwitchGUI();
					break;
				}
				}
			}
		}
	}
}

void App::ShowObjectWindow()
{
	static GFX::Probe::ModelProbe probe;
	cameras.Accept(window.Gfx(), renderer, probe);
	if (ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar))
	{
		static auto currentItem = objects.find("---None---");
		if (ImGui::BeginCombo("Selected object", currentItem->first.c_str()))
		{
			for (auto it = objects.begin(); it != objects.end(); ++it)
			{
				bool selected = (currentItem == it);
				if (ImGui::Selectable(it->first.c_str(), selected))
				{
					ContainerInvoke(currentItem, DisableOutline());
					currentItem = it;
					probe.Reset();
					ContainerInvoke(currentItem, SetOutline());
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		AddModelButton();
		ImGui::SameLine();
		AddLightButton();
		ImGui::SameLine();
		if (ImGui::Button("Delete object"))
			DeleteObject(currentItem);
		ImGui::SameLine();
		ChangeBackgroundButton();
		ImGui::NewLine();
		ContainerInvoke(currentItem, Accept(window.Gfx(), probe));
	}
	ImGui::End();
}

void App::ShowOptionsWindow()
{
	static GFX::Probe::BaseProbe probe;
	if (ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
	{
		if (ImGui::Button("Exit"))
			run = false;
		ImGui::SameLine();
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::SameLine();
		ImGui::Text("%u", ZE_PERF_COUNT("Frame"));
		renderer.ShowWindow(window.Gfx());
	}
	ImGui::End();
}

void App::AddModelButton()
{
	static std::optional<std::string> path = {};
	static std::optional<std::string> error = {};
	static bool contains = false;
	static GFX::Shape::ModelParams params;

	if (const auto file = GUI::DialogWindow::FileBrowserButton("Add model", "Models"))
		path = file;
	if (path)
	{
		if (contains)
		{
			if (GUI::DialogWindow::ShowInfo("Name already present!",
				"Object named \"" + params.name + "\" already exists, enter other unique name.") == DialogResult::Accept)
				contains = false;
		}
		else
		{
			switch (GUI::DialogWindow::GetModelParams(params))
			{
			case DialogResult::Accept:
			{
				if (objects.contains(params.name))
				{
					contains = true;
					break;
				}
				else
				{
					try
					{
						AddShape({ window.Gfx(), renderer, path.value(), params });
					}
					catch (const std::exception& e)
					{
						error = e.what();
					}
				}
			}
			case DialogResult::Cancel:
			{
				path = {};
				contains = false;
				params.Reset();
				break;
			}
			}
		}
	}
	if (error)
	{
		if (GUI::DialogWindow::ShowInfo("Error",
			"Error occured during loading model.\n" + error.value()) == DialogResult::Accept)
			error = {};
	}
}

void App::ChangeBackgroundButton()
{
	static std::optional<std::string> path = {};
	static std::optional<std::string> error = {};

	if (const auto file = GUI::DialogWindow::FileBrowserButton("Change skybox", "Skybox", GUI::DialogWindow::FileType::Directory))
		path = file;
	if (path)
	{
		try
		{
			error = renderer.ChangeSkybox(window.Gfx(), path.value());
		}
		catch (const std::exception& e)
		{
			error = e.what();
		}
		path = {};
	}
	if (error)
	{
		if (GUI::DialogWindow::ShowInfo("Error",
			"Error occured during loading skybox.\n" + error.value()) == DialogResult::Accept)
			error = {};
	}
}

void App::AddLightButton()
{
	static bool active = false;
	static std::optional<std::string> error = {};
	static bool contains = false;
	static GFX::Light::LightParams params;

	if (ImGui::Button("Add light"))
		active = true;
	if (active)
	{
		if (contains)
		{
			if (GUI::DialogWindow::ShowInfo("Name already present!",
				"Object named \"" + params.name + "\" already exists, enter other unique name.") == DialogResult::Accept)
				contains = false;
		}
		else
		{
			switch (GUI::DialogWindow::GetLightParams(params))
			{
			case DialogResult::Accept:
			{
				if (objects.contains(params.name))
				{
					contains = true;
					break;
				}
				else
				{
					try
					{
						switch (params.type)
						{
						case GFX::Light::LightType::Directional:
						{
							AddLight({ window.Gfx(), renderer, std::move(params.name), params.intensity,
								params.color, params.direction });
							break;
						}
						case GFX::Light::LightType::Point:
						{
							AddLight({ window.Gfx(), renderer, std::move(params.name), params.intensity,
								params.color, params.position, params.range, params.size });
							break;
						}
						case GFX::Light::LightType::Spot:
						{
							AddLight({ window.Gfx(), renderer, std::move(params.name), params.intensity,
								params.color, params.position, params.range, params.size,
								params.innerAngle, params.outerAngle, params.direction });
							break;
						}
						}
					}
					catch (const std::exception& e)
					{
						error = e.what();
					}
				}
			}
			case DialogResult::Cancel:
			{
				active = contains = false;
				params.Reset();
				break;
			}
			}
		}
		if (error)
		{
			if (GUI::DialogWindow::ShowInfo("Error",
				"Error occured during creating light.\n" + error.value()) == DialogResult::Accept)
				error = {};
		}
	}
}

void App::MakeFrame()
{
	ZE_PERF_START("Prepare frame");
	window.Gfx().BeginFrame();
	ShowObjectWindow();
	ShowOptionsWindow();
	//ImGui::ShowDemoWindow();
	ZE_PERF_STOP();
	ZE_PERF_START("Submit objects");
	if (cameras.CameraChanged())
		renderer.BindMainCamera(cameras.GetCamera());
	cameras.Submit(RenderChannel::Main);
	for (auto& pointLight : pointLights)
		pointLight.Submit(RenderChannel::Main | RenderChannel::Light);
	for (auto& spotLight : spotLights)
		spotLight.Submit(RenderChannel::Main | RenderChannel::Light);
	for (auto& directionalLight : directionalLights)
		directionalLight.Submit(RenderChannel::Main | RenderChannel::Light);
	for (auto& model : models)
		model.Submit(RenderChannel::Main | RenderChannel::Shadow);
	for (auto& shape : shapes)
		shape->Submit(RenderChannel::Main | RenderChannel::Shadow);
	ZE_PERF_STOP();
	ZE_PERF_START("Rendering");
	renderer.Execute(window.Gfx());
	renderer.Reset();
	window.Gfx().EndFrame();
}

App::App(const std::string& commandLine)
	: window(WINDOW_TITLE, 1920, 1080), renderer(window.Gfx(), "Skybox/Space", ".png"),
	cameras(std::make_unique<Camera::PersonCamera>(window.Gfx(), renderer,
		Camera::CameraParams({ -8.0f, 0.0f, 0.0f }, "Main camera", Math::ToRadians(90.0f), 0.0f, 1.047f, 0.01f, 500.0f)))
{
	window.Gfx().Gui().SetFont("Fonts/Arial.ttf", 14.0f);
	objects.emplace("---None---", std::make_pair(Container::None, 0));
	// Debug scene
	AddLight({ window.Gfx(), renderer, "Light bulb", 1.0f, ColorF3(1.0f, 1.0f, 1.0f), Float3(-20.0f, 2.0f, -4.0f), 50 });
	GFX::Shape::ModelParams params({ 0.0f, -8.2f, 6.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, "Nanosuit", 0.70f);
	AddShape({ window.Gfx(), renderer, "Models/nanosuit/nanosuit.obj", params });
	params = { Float3(-5.0f, -2.0f, 7.0f), Float4(0.0f, 0.0f, 0.0f, 1.0f), "Wall", 2.0f };
	AddShape({ window.Gfx(), renderer, "Models/bricks/brick_wall.obj", params });

	// Sample Scene
#ifndef _ZE_MODE_DEBUG
	cameras.AddCamera(std::make_unique<Camera::PersonCamera>(window.Gfx(), renderer,
		Camera::CameraParams({ 0.0f, 40.0f, -4.0f }, "Camera", 0.0f, Math::ToRadians(45.0f), 1.047f, 2.0f, 15.0f)));
	AddLight({ window.Gfx(), renderer, "Pumpkin candle", 5.0f, ColorF3(1.0f, 0.96f, 0.27f), Float3(14.0f, -6.3f, -5.0f), 85 });
	AddLight({ window.Gfx(), renderer, "Torch", 5.0f, ColorF3(1.0f, 0.0f, 0.2f), Float3(21.95f, -1.9f, 9.9f), 70 });
	AddLight({ window.Gfx(), renderer, "Blue ilumination", 10.0f, ColorF3(0.0f, 0.46f, 1.0f), Float3(43.0f, 27.0f, 1.8f), 70 });
	Float3 direction = { -0.64f, -1.0f, 0.5f };
	AddLight({ window.Gfx(), renderer, "Space light", 8.0f, ColorF3(1.3f, 2.3f, 1.3f),
		Float3(7.5f, 60.0f, -5.0f), 126, 2.0f, Math::ToRadians(15.0f), Math::ToRadians(24.5f), Math::NormalizeStore(direction) });
	direction = { -1.0f, 1.0f, -0.7f };
	AddLight({ window.Gfx(), renderer,"Lion flare", 9.0f, ColorF3(0.8f, 0.0f, 0.8f),
		Float3(-61.0f, -6.0f, 5.0f), 150, 1.0f, Math::ToRadians(35.0f), Math::ToRadians(45.0f), Math::NormalizeStore(direction) });
	direction = { -0.6f, 0.75f, 0.3f };
	AddLight({ window.Gfx(), renderer,"Dragon flame", 3.0f, ColorF3(0.04f, 0.0f, 0.52f),
		Float3(-35.0f, -8.0f, 2.0f), 175, 0.5f, Math::ToRadians(27.0f), Math::ToRadians(43.0f), Math::NormalizeStore(direction) });
	direction = { 0.0f, -0.7f, -0.7f };
	AddLight({ window.Gfx(), renderer, "Moon", 0.1f, ColorF3(0.7608f, 0.7725f, 0.8f), Math::NormalizeStore(direction) });
	params = { Float3(0.0f, -8.0f, 0.0f), Float4(0.0f, 0.0f, 0.0f, 1.0f), "Sponza", 0.045f };
	AddShape({ window.Gfx(), renderer, "Models/Sponza/sponza.obj", params });
	params = { Float3(13.5f, -8.2f, -5.0f), Float4(0.0f, 0.0f, 0.0f, 1.0f), "Jack O'Lantern", 13.00f };
	AddShape({ window.Gfx(), renderer, "Models/Jack/Jack_O_Lantern.3ds", params });
	params = { Float3(-39.0f, -8.1f, 2.0f), Float4(0.0f, 0.0f, 0.0f, 1.0f), "Black Dragon", 0.15f };
	Math::XMStoreFloat4(&params.rotation, Math::XMQuaternionRotationRollPitchYaw(0.0f, Math::ToRadians(290.0f), 0.0f));
	AddShape({ window.Gfx(), renderer, "Models/Black Dragon/Dragon 2.5.fbx", params });
	params = { Float3(-20.0f, 0.0f, -6.0f), Float4(0.0f, 0.0f, 0.0f, 1.0f), "Sting Sword", 0.2f };
	Math::XMStoreFloat4(&params.rotation,
		Math::XMQuaternionRotationRollPitchYaw(Math::ToRadians(35.0f), Math::ToRadians(270.0f), Math::ToRadians(110.0f)));
	AddShape({ window.Gfx(), renderer, "Models/Sting_Sword/Sting_Sword.obj", params });
	params = { Float3(41.6f, 18.5f, 8.5f), Float4(0.0f, 0.0f, 0.0f, 1.0f), "TIE", 3.6f };
	Math::XMStoreFloat4(&params.rotation,
		Math::XMQuaternionRotationRollPitchYaw(0.0f, Math::ToRadians(87.1f), Math::ToRadians(301.0f)));
	AddShape({ window.Gfx(), renderer, "Models/tie/tie.obj", params });
#endif
}

int App::Run()
{
	std::pair<bool, int> status;
	while (run)
	{
		ZE_PERF_START("Frame");
		ZE_PERF_START("Input processing");
		status = WinAPI::Window::ProcessMessage();
		if (status.first)
			return status.second;
		ProcessInput();
		ZE_PERF_STOP();
		MakeFrame();
		ZE_PERF_STOP();
		if (ZE_PERF_COUNT("Frame") == 100000)
			break;
	}
	return 0;
}