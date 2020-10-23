#include "CameraPool.h"

namespace Camera
{
	void CameraPool::ProcessInput(WinAPI::Window& window) noexcept
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
				case WinAPI::Mouse::Event::Type::WheelForward:
				{
					if (!window.IsCursorEnabled() && moveSpeed <= MAX_MOVE_SPEED - 0.01f - FLT_EPSILON)
						moveSpeed += 0.01f;
					break;
				}
				case WinAPI::Mouse::Event::Type::WheelBackward:
				{
					if (!window.IsCursorEnabled() && moveSpeed >= 0.012f + FLT_EPSILON)
						moveSpeed -= 0.01f;
					break;
				}
				case WinAPI::Mouse::Event::Type::RawMove:
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
		if (window.Keyboard().IsKeyDown(VK_LEFT))
			camera.Roll(rollSpeed);
		if (window.Keyboard().IsKeyDown(VK_RIGHT))
			camera.Roll(-rollSpeed);
	}

	bool CameraPool::AddCamera(std::unique_ptr<ICamera> camera) noexcept
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

	void CameraPool::Submit(uint64_t channelFilter) noexcept
	{
		for (auto it = cameras.begin(); it != cameras.end(); ++it)
			if (it->first != active)
				it->second->Submit(channelFilter);
	}

	bool CameraPool::Accept(GFX::Graphics& gfx, GFX::Probe::BaseProbe& probe) noexcept
	{
		ImGui::SliderFloat("Move speed", &moveSpeed, 0.001f, MAX_MOVE_SPEED, "%.3f");
		ImGui::SliderFloat("Roll speed", &rollSpeed, 0.01f, 0.5f, "%.2f");
		ImGui::SliderFloat("Camera speed", &rotateSpeed, 1.0f, 5.0f, "%.1f");
		static std::map<std::string, std::unique_ptr<ICamera>>::iterator currentItem = cameras.find(active);
		if (ImGui::BeginCombo("Active camera", currentItem->first.c_str()))
		{
			for (auto it = cameras.begin(); it != cameras.end(); ++it)
			{
				bool selected = (currentItem == it);
				if (ImGui::Selectable(it->first.c_str(), selected))
				{
					currentItem = it;
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Select camera") && currentItem->first != active)
		{
			active = currentItem->first;
			currentItem->second->Reset();
			cameraChanged = true;
		}
		ImGui::NewLine();
		const auto& cameraPos = currentItem->second->GetPos();
		ImGui::Text("Position: [%.3f, %.3f, %.3f]", cameraPos.x, cameraPos.y, cameraPos.z);
		return currentItem->second->Accept(gfx, probe);
	}
}