#include "Camera/Cameras.h"
#include "GUI/DialogWindow.h"

namespace ZE::Camera
{
	void CameraPool::ProcessInput(Window::MainWindow& window) noexcept
	{
		auto& camera = GetCamera();
		while (window.Mouse().IsInput())
		{
			if (auto opt = window.Mouse().Read())
			{
				const auto& value = opt.value();
				if (value.IsRightDown() && window.IsCursorEnabled())
					camera.Rotate(rotateSpeed * static_cast<float>(value.GetDY()) / window.Gfx().GetHeight(),
						rotateSpeed * static_cast<float>(value.GetDX()) / window.Gfx().GetWidth());
				switch (value.GetType())
				{
				case Window::Mouse::Event::Type::WheelForward:
				{
					if (!window.IsCursorEnabled() && moveSpeed <= MAX_MOVE_SPEED - 0.01f - FLT_EPSILON)
						moveSpeed += 0.01f;
					break;
				}
				case Window::Mouse::Event::Type::WheelBackward:
				{
					if (!window.IsCursorEnabled() && moveSpeed >= 0.012f + FLT_EPSILON)
						moveSpeed -= 0.01f;
					break;
				}
				case Window::Mouse::Event::Type::RawMove:
				{
					if (!window.IsCursorEnabled())
						camera.Rotate(rotateSpeed * static_cast<float>(value.GetDY()) / window.Gfx().GetHeight(),
							rotateSpeed * static_cast<float>(value.GetDX()) / window.Gfx().GetWidth());
					break;
				}
				}
			}
		}
		if (window.Keyboard().IsKeyDown('W'))
			camera.MoveZ(moveSpeed);
		if (window.Keyboard().IsKeyDown('S'))
			GetCamera().MoveZ(-moveSpeed);
		if (window.Keyboard().IsKeyDown('A'))
			camera.MoveX(-moveSpeed);
		if (window.Keyboard().IsKeyDown('D'))
			camera.MoveX(moveSpeed);
		if (window.Keyboard().IsKeyDown(VK_SPACE))
			camera.MoveY(moveSpeed);
		if (window.Keyboard().IsKeyDown('C'))
			camera.MoveY(-moveSpeed);
		if (window.Keyboard().IsKeyDown('Q'))
			camera.Roll(rollSpeed);
		if (window.Keyboard().IsKeyDown('E'))
			camera.Roll(-rollSpeed);
	}

	bool CameraPool::AddCamera(std::unique_ptr<ICamera>&& camera) noexcept
	{
		if (!cameras.contains(camera->GetName()))
		{
			cameras.emplace(camera->GetName(), std::move(camera));
			return true;
		}
		return false;
	}

	bool CameraPool::DeleteCamera(const std::string& name) noexcept
	{
		if (name == active || cameras.size() == 1)
			return false;
		cameras.erase(name);
		return true;
	}

	void CameraPool::Submit(U64 channelFilter) noexcept
	{
		for (auto it = cameras.begin(); it != cameras.end(); ++it)
			if (it->first != active)
				it->second->Submit(channelFilter);
	}

	bool CameraPool::Accept(GFX::Graphics& gfx, GFX::Pipeline::RenderGraph& graph, GFX::Probe::BaseProbe& probe) noexcept
	{
		static bool errorDelete = false;
		static bool errorAdd = false;
		static bool add = false;
		static CameraParams params;

		bool change = false;
		if (ImGui::Begin("Camera control", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
		{
			if (ImGui::Button("Add camera"))
				add = true;
			if (add)
			{
				if (errorAdd)
				{
					if (GUI::DialogWindow::ShowInfo("Name already present!",
						"Camera named \"" + params.name + "\" already exists, enter other unique name.") == DialogResult::Accept)
						errorAdd = false;
				}
				else
				{
					switch (GUI::DialogWindow::GetCameraParams(params))
					{
					case DialogResult::Accept:
					{
						switch (params.type)
						{
						case CameraType::Person:
						{
							errorAdd = !AddCamera(std::make_unique<PersonCamera>(gfx, graph, std::move(params)));
							break;
						}
						case CameraType::Floating:
						{
							errorAdd = !AddCamera(std::make_unique<FloatingCamera>(gfx, graph, std::move(params)));
							break;
						}
						}
						if (errorAdd)
							break;
					}
					case DialogResult::Cancel:
					{
						add = false;
						params.Reset();
						break;
					}
					}
				}
			}
			ImGui::SameLine();
			ImGui::Text("Active camera: ");
			ImGui::SameLine();
			ImGui::TextColored({ 0.4f, 1.0f, 0.6f, 1.0f }, active.c_str());
			static std::unordered_map<std::string, std::unique_ptr<ICamera>>::iterator currentItem = cameras.find(active);
			if (ImGui::BeginCombo("Selected camera", currentItem->first.c_str()))
			{
				for (auto it = cameras.begin(); it != cameras.end(); ++it)
				{
					bool selected = (currentItem == it);
					if (ImGui::Selectable(it->first.c_str(), selected))
					{
						currentItem->second->DisableOutline();
						currentItem = it;
						currentItem->second->SetOutline();
					}
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::NewLine();

			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::Button("Set active") && currentItem->first != active)
			{
				active = currentItem->first;
				currentItem->second->Reset();
				cameraChanged = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete"))
			{
				if (DeleteCamera(currentItem->first))
				{
					currentItem = cameras.find(active);
					cameraChanged = true;
				}
				else
					errorDelete = true;
			}
			if (errorDelete)
				if (GUI::DialogWindow::ShowInfo("Error", "Cannot delete active camera!") == DialogResult::Accept)
					errorDelete = false;
			ImGui::SameLine();
			const auto& cameraPos = currentItem->second->GetPos();
			ImGui::Text("Position: [%.2f, %.2f, %.2f]", cameraPos.x, cameraPos.y, cameraPos.z);

			ImGui::Columns(3, "##camera_options", false);
			ImGui::Text("Move speed");
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::SliderFloat("##move_speed", &moveSpeed, 0.001f, MAX_MOVE_SPEED, "%.3f");
			ImGui::NextColumn();
			ImGui::Text("Look speed");
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::SliderFloat("##look_speed", &rotateSpeed, 1.0f, 5.0f, "%.1f");
			ImGui::NextColumn();
			ImGui::Text("Roll speed");
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::SliderFloat("##roll_spedd", &rollSpeed, 0.01f, 0.5f, "%.2f");
			ImGui::NextColumn();

			change = currentItem->second->Accept(gfx, probe);
			ImGui::Columns(1);
		}
		ImGui::End();
		return change;
	}
}