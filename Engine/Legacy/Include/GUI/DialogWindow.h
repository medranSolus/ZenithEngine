#pragma once
#include "GFX/Shape/ModelParams.h"
#include "GFX/Light/LightParams.h"
#include "Camera/CameraParams.h"
#include <optional>
#include <filesystem>

namespace ZE::GUI
{
	class DialogWindow
	{
	public:
		enum class Result : U8 { Accept, Cancel, None };
		enum class FileType : U8 { Model, Image, Directory };

	private:
		static inline std::vector<std::string> drives;

		static void SetupDrives() noexcept;
		static bool IsCorrectExtention(const std::filesystem::directory_entry& entry, FileType searchType) noexcept;

	public:
		DialogWindow() = delete;

		static std::vector<std::filesystem::directory_entry> GetDirContent(const std::filesystem::directory_entry& entry, FileType searchType) noexcept;
		static std::optional<std::string> FileBrowserButton(const std::string& title, const std::string& startDir, FileType searchType = FileType::Model) noexcept;
		static Result GetModelParams(GFX::Shape::ModelParams& params) noexcept;
		static Result GetLightParams(GFX::Light::LightParams& params) noexcept;
		static Result GetCameraParams(Camera::CameraParams& params) noexcept;
		static Result ShowInfo(const std::string& title, const std::string& text) noexcept;
	};
}

typedef ZE::GUI::DialogWindow::Result DialogResult;