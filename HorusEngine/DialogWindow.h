#pragma once
#include "ModelParams.h"
#include "LightParams.h"
#include "CameraParams.h"
#include <optional>
#include <filesystem>

namespace GFX::GUI
{
	class DialogWindow
	{
	public:
		enum class Result : uint8_t { Accept, Cancel, None };
		enum class FileType : uint8_t { Model, Image, Directory };

	private:
		static std::vector<std::string> drives;

		static void SetupDrives() noexcept;
		static bool IsCorrectExtention(const std::filesystem::directory_entry& entry, FileType searchType) noexcept;

	public:
		DialogWindow() = delete;
		~DialogWindow() = default;

		static std::vector<std::filesystem::directory_entry> GetDirContent(const std::filesystem::directory_entry& entry, FileType searchType) noexcept;
		static std::optional<std::string> FileBrowserButton(const std::string& title, const std::string& startDir, FileType searchType = FileType::Model) noexcept;
		static Result GetModelParams(Shape::ModelParams& params) noexcept;
		static Result GetLightParams(Light::LightParams& params) noexcept;
		static Result GetCameraParams(Camera::CameraParams& params) noexcept;
		static Result ShowInfo(const std::string& title, const std::string& text) noexcept;
	};
}

typedef GFX::GUI::DialogWindow::Result DialogResult;