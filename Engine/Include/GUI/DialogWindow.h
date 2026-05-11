#pragma once

namespace ZE::GUI::DialogWindow
{
	// Type of file to look for when searching disk
	enum class FileType : U8 { All, Directory, Image, ResourcePack, Model };

	// Get list of files in given directory
	Expected<std::vector<std::filesystem::directory_entry>> GetDirContent(const std::filesystem::directory_entry& entry, FileType searchType) noexcept;
	// Create button with pop-up window allowing for navigating through files on disk
	Expected<std::optional<std::string>> FileBrowserButton(std::string_view title, std::string_view startDir, FileType searchType = FileType::All) noexcept;
	// Show simple pop-up window
	bool ShowInfo(std::string_view title, std::string_view text) noexcept;
}