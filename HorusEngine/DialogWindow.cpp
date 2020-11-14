#include "DialogWindow.h"
#include "Math.h"
#include "WinApi.h"
#include "Utils.h"
#include "Surface.h"
#include "ImGui\imgui.h"
#include "ImGui\imgui_stdlib.h"
#include <algorithm>

namespace GFX::GUI
{
	std::vector<std::string> DialogWindow::drives;

	void DialogWindow::SetupDrives() noexcept
	{
		DWORD driveMask = GetLogicalDrives();
		std::error_code error;
		std::string drive = "A:";
		for (; drive.front() <= 'Z'; ++drive.front())
		{
			if ((driveMask & 1) && std::filesystem::exists(drive + "\\", error))
				drives.emplace_back(drive);
			driveMask >>= 1;
		}
	}

	bool DialogWindow::IsCorrectExtention(const std::filesystem::directory_entry& entry, FileType searchType) noexcept
	{
		if (searchType != FileType::Image && entry.is_directory())
			return true;
		if (searchType != FileType::Directory)
		{
			const std::string ext = entry.path().extension().string();
			switch (searchType)
			{
			case FileType::Model:
				return ext == ".fbx" ||
					ext == ".3ds" ||
					ext == ".obj";
			case FileType::Image:
				return Surface::IsImage(ext);
			}
		}
		return false;
	}

	std::vector<std::filesystem::directory_entry> DialogWindow::GetDirContent(const std::filesystem::directory_entry& entry, FileType searchType) noexcept
	{
		std::vector<std::filesystem::directory_entry> dirContent;
		for (const auto& entry : std::filesystem::directory_iterator(entry, std::filesystem::directory_options::skip_permission_denied))
			if (IsCorrectExtention(entry, searchType))
				dirContent.emplace_back(entry);
		std::sort(dirContent.begin(), dirContent.end(), [&searchType](const std::filesystem::directory_entry& e1, const std::filesystem::directory_entry& e2)
			{
				// Ascending
				if (searchType != FileType::Directory)
				{
					if (e1.is_directory() && !e2.is_directory())
						return true;
					else if (!e1.is_directory() && e2.is_directory())
						return false;
				}
				return e1.path().filename() < e2.path().filename();
			});
		return std::move(dirContent);
	}

	std::optional<std::string> DialogWindow::FileBrowserButton(const std::string& title, const std::string& startDir, FileType searchType) noexcept
	{
		static std::filesystem::directory_entry currentDir;
		static size_t selected;
		if (ImGui::Button(title.c_str()))
		{
			currentDir = std::move(std::filesystem::directory_entry(std::filesystem::current_path().string() + "\\" + startDir));
			selected = -1;
			if (drives.size() == 0)
				SetupDrives();
			ImGui::OpenPopup(title.c_str());
		}

		// Always center this window when appearing
		ImGui::SetNextWindowPos({ ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f },
			ImGuiCond_Appearing, { 0.5f, 0.5f });
		std::string selectFile = "";
		if (ImGui::BeginPopupModal(title.c_str(), nullptr/*, ImGuiWindowFlags_NoMove*/))
		{
			if (!currentDir.exists())
				currentDir = std::move(std::filesystem::directory_entry(std::filesystem::current_path()));
			std::vector<std::filesystem::directory_entry> dirContent = std::move(GetDirContent(currentDir, searchType));

			if (ImGui::Button(" ^ "))
			{
				currentDir = std::move(std::filesystem::directory_entry(currentDir.path().parent_path()));
				selected = -1;
			}
			ImGui::SameLine();
			std::string driveLetter = currentDir.path().root_name().string();
			ImGui::SetNextItemWidth(40.0f);
			if (ImGui::BeginCombo("##dialog_drive", currentDir.path().root_name().string().c_str()))
			{
				for (const auto& letter : drives)
				{
					bool selected = (driveLetter == letter);
					if (ImGui::Selectable(letter.c_str(), selected))
						currentDir = std::move(std::filesystem::directory_entry(letter + "\\"));
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();

			ImGui::BeginChild("##dialog_path", { -1.0f, 34.0f }, false, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Dummy({ 0.0f, 0.0f });
			auto path = currentDir.path().wstring();
			path.erase(path.begin(), path.begin() + 3);
			ImGui::Text(Utils::ToAscii(path).c_str());
			ImGui::EndChild();

			ImGui::BeginChild("##dialog_content", { -1.0f, -35.0f }, true);
			for (size_t i = 0, size = dirContent.size(); i < size; ++i)
			{
				if (ImGui::Selectable(Utils::ToAscii(dirContent.at(i).path().filename().wstring()).c_str(), selected == i, ImGuiSelectableFlags_AllowDoubleClick))
				{
					selected = i;
					if (ImGui::IsMouseDoubleClicked(0))
					{
						if (dirContent.at(i).is_directory())
						{
							currentDir = std::move(dirContent.at(i));
							selected = -1;
						}
					}
				}
			}
			ImGui::EndChild();

			ImGui::BeginChild("##dialog_selected", { -107.0f, 30.0f }, true);
			if (selected != -1)
				ImGui::Text(Utils::ToAscii(dirContent.at(selected).path().filename().wstring()).c_str());
			ImGui::EndChild();
			ImGui::SameLine();
			if (ImGui::Button("Select", { 0.0f, 30.0f }))
			{
				selectFile = Utils::ToAscii(dirContent.at(selected).path().wstring());
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", { 0.0f, 30.0f }))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		if (selectFile.size())
			return selectFile;
		return {};
	}

	DialogWindow::Result DialogWindow::GetModelParams(Shape::ModelParams& params) noexcept
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

	DialogWindow::Result DialogWindow::GetLightParams(Light::LightParams& params) noexcept
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
				for (uint8_t i = 0; i <= Light::LightType::Spot; ++i)
				{
					bool selected = (params.type == i);
					if (ImGui::Selectable(TYPES[i], selected))
						params.type = static_cast<Light::LightType>(i);
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

			if (params.type == Light::LightType::Directional || params.type == Light::LightType::Spot)
			{
				if (ImGui::SliderFloat3("Direction", reinterpret_cast<float*>(&params.direction), -1.0f, 1.0f, "%.2f"))
					Math::NormalizeStore(params.direction);
			}
			if (params.type == Light::LightType::Point || params.type == Light::LightType::Spot)
			{
				ImGui::InputFloat3("Position", reinterpret_cast<float*>(&params.position), "%.2f");
				static const size_t STEP = 1;
				ImGui::InputScalar("Range", ImGuiDataType_U64, &params.range, &STEP);
				ImGui::InputFloat("Indicator size", &params.size, 0.01f, 0.0f, "%.3f");
				if (params.size < 0.001f)
					params.size = 0.001f;
				if (params.type == Light::LightType::Spot)
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
				for (uint8_t i = 0; i <= Camera::CameraType::Floating; ++i)
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

	DialogWindow::Result DialogWindow::ShowInfo(const std::string& title, const std::string& text) noexcept
	{
		Result result = Result::None;
		if (!ImGui::IsPopupOpen(title.c_str()))
			ImGui::OpenPopup(title.c_str());
		ImGui::SetNextWindowPos({ ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f },
			ImGuiCond_Appearing, { 0.5f, 0.5f });
		if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::PushTextWrapPos(250.0f);
			ImGui::TextWrapped(text.c_str());
			ImGui::PopTextWrapPos();
			if (ImGui::Button("OK", { -1.0f, 0.0f }))
			{
				result = Result::Accept;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return result;
	}
}