#include "App.h"
#include "GUI/DialogWindow.h"

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