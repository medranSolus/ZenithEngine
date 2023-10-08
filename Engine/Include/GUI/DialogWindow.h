#pragma once

namespace ZE::GUI::DialogWindow
{
	enum class FileType : U8 { All, Directory, Image, ResourcePack, Model };

	std::vector<std::filesystem::directory_entry> GetDirContent(const std::filesystem::directory_entry& entry, FileType searchType);
	std::optional<std::string> FileBrowserButton(std::string_view title, std::string_view startDir, FileType searchType = FileType::All);
}