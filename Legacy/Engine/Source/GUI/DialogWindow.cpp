#include "GUI/DialogWindow.h"
#include "Platform/WinAPI/WinApi.h"
#include "GFX/Surface.h"
#include "imgui.h"
#include "imgui_stdlib.h"

namespace ZE::GUI
{
	DialogWindow::Result DialogWindow::GetModelParams(GFX::Shape::ModelParams& params) noexcept
	{
		static constexpr const char* TITLE = "Input model parameters";
		Result result = Result::None;

		if (!ImGui::IsPopupOpen(TITLE))
			ImGui::OpenPopup(TITLE);
		ImGui::SetNextWindowPos({ ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f },
			ImGuiCond_Appearing, { 0.5f, 0.5f });

		if (ImGui::BeginPopupModal(TITLE, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::InputText("Name", &params.name);
			ImGui::InputFloat("Scale", &params.scale, 0.01f, 0.0f, "%.3f");
			if (params.scale < 0.001f)
				params.scale = 0.001f;

			ImGui::Columns(2, "##params_position", false);
			ImGui::Text("Position");
			ImGui::SetNextItemWidth(-15.0f);
			ImGui::InputFloat("X##pos", &params.position.x, 0.1f, 0.0f, "%.2f");
			ImGui::SetNextItemWidth(-15.0f);
			ImGui::InputFloat("Y##pos", &params.position.y, 0.1f, 0.0f, "%.2f");
			ImGui::SetNextItemWidth(-15.0f);
			ImGui::InputFloat("Z##pos", &params.position.z, 0.1f, 0.0f, "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Rotation");
			ImGui::SetNextItemWidth(-15.0f);
			ImGui::SliderAngle("X##rot", &params.rotation.x, 0.0f, 360.0f, "%.2f");
			ImGui::SetNextItemWidth(-15.0f);
			ImGui::SliderAngle("Y##rot", &params.rotation.y, 0.0f, 360.0f, "%.2f");
			ImGui::SetNextItemWidth(-15.0f);
			ImGui::SliderAngle("Z##rot", &params.rotation.z, 0.0f, 360.0f, "%.2f");
			ImGui::Columns(1);

			if (ImGui::Button("Accept", { 135.0f, 0.0f }))
			{
				Math::XMStoreFloat4(&params.rotation,
					Math::XMQuaternionRotationRollPitchYaw(params.rotation.x, params.rotation.y, params.rotation.z));
				result = Result::Accept;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", { 135.0f, 0.0f }))
			{
				result = Result::Cancel;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return result;
	}

	DialogWindow::Result DialogWindow::GetLightParams(GFX::Light::LightParams& params) noexcept
	{
		static constexpr const char* TITLE = "Input light parameters";
		static constexpr const char* TYPES[] = { "Directional light", "Point light", "Spot light" };
		Result result = Result::None;

		if (!ImGui::IsPopupOpen(TITLE))
			ImGui::OpenPopup(TITLE);
		ImGui::SetNextWindowPos({ ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f },
			ImGuiCond_Appearing, { 0.5f, 0.5f });

		if (ImGui::BeginPopupModal(TITLE, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
		{
			if (ImGui::BeginCombo("Light type", TYPES[params.type]))
			{
				for (U8 i = 0; i <= GFX::Light::LightType::Spot; ++i)
				{
					bool selected = (params.type == i);
					if (ImGui::Selectable(TYPES[i], selected))
						params.type = static_cast<GFX::Light::LightType>(i);
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::InputText("Name", &params.name);
			ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&params.color), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
			ImGui::InputFloat("Intensity", &params.intensity, 0.001f, 0.0f, "%.3f");
			if (params.intensity < 0.0f)
				params.intensity = 0.0f;

			if (params.type == GFX::Light::LightType::Directional || params.type == GFX::Light::LightType::Spot)
			{
				if (ImGui::SliderFloat3("Direction", reinterpret_cast<float*>(&params.direction), -1.0f, 1.0f, "%.2f"))
					Math::NormalizeStore(params.direction);
			}
			if (params.type == GFX::Light::LightType::Point || params.type == GFX::Light::LightType::Spot)
			{
				ImGui::InputFloat3("Position", reinterpret_cast<float*>(&params.position), "%.2f");
				static const U64 STEP = 1;
				ImGui::InputScalar("Range", ImGuiDataType_U64, &params.range, &STEP);
				ImGui::InputFloat("Indicator size", &params.size, 0.01f, 0.0f, "%.3f");
				if (params.size < 0.001f)
					params.size = 0.001f;
				if (params.type == GFX::Light::LightType::Spot)
				{
					if (ImGui::SliderAngle("Outer angle", &params.outerAngle, 0.0f, 45.0f, "%.2f"))
					{
						if (params.innerAngle > params.outerAngle)
							params.innerAngle = params.outerAngle;
					}
					ImGui::SliderAngle("Inner angle", &params.innerAngle, 0.0f, Math::ToDegrees(params.outerAngle), "%.2f");
				}
			}

			if (ImGui::Button("Accept", { 147.0f, 0.0f }))
			{
				result = Result::Accept;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", { 147.0f, 0.0f }))
			{
				result = Result::Cancel;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return result;
	}

	DialogWindow::Result DialogWindow::GetCameraParams(Camera::CameraParams& params) noexcept
	{
		static constexpr const char* TITLE = "Input camera parameters";
		static constexpr const char* TYPES[] = { "Person", "Floating" };
		Result result = Result::None;

		if (!ImGui::IsPopupOpen(TITLE))
			ImGui::OpenPopup(TITLE);
		ImGui::SetNextWindowPos({ ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f },
			ImGuiCond_Appearing, { 0.5f, 0.5f });

		if (ImGui::BeginPopupModal(TITLE, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
		{
			if (ImGui::BeginCombo("Camera type", TYPES[params.type]))
			{
				for (U8 i = 0; i <= Camera::CameraType::Floating; ++i)
				{
					bool selected = (params.type == i);
					if (ImGui::Selectable(TYPES[i], selected))
						params.type = static_cast<Camera::CameraType>(i);
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::InputText("Name", &params.name);
			ImGui::InputFloat3("Position", reinterpret_cast<float*>(&params.position), "%.2f");
			ImGui::SliderAngle("FOV", &params.fov, 1.0f, 179.0f, "%.1f");
			ImGui::InputFloat("Near clip", &params.nearClip, 0.01f, 0.0f, "%.3f");
			if (params.nearClip < 0.01f)
				params.nearClip = 0.01f;
			else if (params.nearClip > 10.0f)
				params.nearClip = 10.0f;
			ImGui::InputFloat("Far clip", &params.farClip, 0.1f, 0.0f, "%.1f");
			if (params.farClip < params.nearClip + 0.01f)
				params.farClip = params.nearClip + 0.01f;
			else if (params.farClip > 50000.0f)
				params.farClip = 50000.0f;
			ImGui::SliderAngle("Vertical angle", &params.angleVertical, 0.0f, 179.0f, "%.2f");
			ImGui::SliderAngle("Horizontal angle", &params.angleHorizontal, 0.0f, 359.0f, "%.2f");

			if (ImGui::Button("Accept", { 147.0f, 0.0f }))
			{
				result = Result::Accept;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", { 147.0f, 0.0f }))
			{
				result = Result::Cancel;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return result;
	}
}