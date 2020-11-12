#pragma once
#include "ModelParams.h"
#include "LightParams.h"
#include <optional>
#include <filesystem>

namespace GFX::GUI
{
	class DialogWindow
	{
	public:
		enum class Result { Accept, Cancel, None };

	private:
		static std::vector<std::string> drives;

		static void SetupDrives() noexcept;
		static bool IsCorrectExtention(const std::filesystem::directory_entry& entry) noexcept;
		static std::vector<std::filesystem::directory_entry> GetDirContent(const std::filesystem::directory_entry& entry) noexcept;

	public:
		DialogWindow() = delete;
		~DialogWindow() = default;

		static std::optional<std::string> FileBrowserButton(const std::string& title, const std::string& startDir);
		static Result GetModelParams(Shape::ModelParams& params);
		static Result GetLightParams(Light::LightParams& params);
		static Result ShowInfo(const std::string& title, const std::string& text);
	};
}

typedef GFX::GUI::DialogWindow::Result DialogResult;