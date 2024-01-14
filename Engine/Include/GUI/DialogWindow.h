#pragma once

namespace ZE::GUI::DialogWindow
{
	// Type of file to look for when searching disk
	enum class FileType : U8 { All, Directory, Image, ResourcePack, Model };

	// Get list of files in given directory
	std::vector<std::filesystem::directory_entry> GetDirContent(const std::filesystem::directory_entry& entry, FileType searchType);
	// Create button with pop-up window allowing for navigating through files on disk
	std::optional<std::string> FileBrowserButton(std::string_view title, std::string_view startDir, FileType searchType = FileType::All);
}