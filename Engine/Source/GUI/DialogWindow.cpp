#include "GUI/DialogWindow.h"
#include "GUI/DearImGui.h"
#include "Data/AssetsStreamer.h"

namespace ZE::GUI::DialogWindow
{
	bool IsCorrectExtention(const std::filesystem::directory_entry& entry, FileType searchType) noexcept
	{
		if (entry.is_directory())
			return true;
		switch (searchType)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ZE::GUI::DialogWindow::FileType::All:
			return true;
		case ZE::GUI::DialogWindow::FileType::Directory:
			return false;
		case FileType::Image:
		{
			if (entry.path().has_extension())
			{
				static constexpr std::array<const char*, 14> EXTENSIONS =
				{
					".png", ".jpg", ".jpeg", ".bmp", ".hdp", ".jxr",
					".ico", ".tif", ".tiff", ".gif", ".wdp", ".dds",
					".hdr", ".tga"
				};
				std::filesystem::path currentExt = entry.path().extension();
				for (auto ext : EXTENSIONS)
					if (!currentExt.compare(ext))
						return true;
			}
			break;
		}
		case ZE::GUI::DialogWindow::FileType::ResourcePack:
			return entry.path().has_extension() && !entry.path().extension().compare(Data::AssetsStreamer::RESOURCE_FILE_EXT);
		case ZE::GUI::DialogWindow::FileType::Model:
		{
			if (entry.path().has_extension())
			{
				static constexpr std::array<const char*, 3> EXTENSIONS =
				{
					".fbx", ".3ds", ".obj"
				};
				std::filesystem::path currentExt = entry.path().extension();
				for (auto ext : EXTENSIONS)
					if (!currentExt.compare(ext))
						return true;
			}
			break;
		}
		}
		return false;
	}

	std::vector<std::filesystem::directory_entry> GetDirContent(const std::filesystem::directory_entry& entry, FileType searchType)
	{
		std::vector<std::filesystem::directory_entry> dirContent;
		for (const auto& e : std::filesystem::directory_iterator(entry, std::filesystem::directory_options::skip_permission_denied))
			if (IsCorrectExtention(e, searchType))
				dirContent.emplace_back(e);

		std::sort(dirContent.begin(), dirContent.end(), [searchType](const std::filesystem::directory_entry& e1, const std::filesystem::directory_entry& e2)
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
		return dirContent;
	}

	std::optional<std::string> FileBrowserButton(std::string_view title, std::string_view startDir, FileType searchType)
	{
		static std::filesystem::directory_entry currentDir;
		static U64 selected = UINT64_MAX;
		auto setCurrentDir = [](const std::filesystem::path& newPath)
			{
				currentDir.assign(newPath);
				selected = UINT64_MAX;
			};

		// Button for opening pop-up window
		if (ImGui::Button(title.data()))
		{
			setCurrentDir(std::filesystem::current_path().append(startDir));
			ImGui::OpenPopup(title.data());
		}

		// Always center this window when appearing
		ImGui::SetNextWindowPos({ ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f },
			ImGuiCond_Appearing, { 0.5f, 0.5f });
		std::string selectedFile = "";
		if (ImGui::BeginPopupModal(title.data()))
		{
			if (!currentDir.exists())
				setCurrentDir(std::filesystem::current_path());

			// Move up in directory structure
			if (ImGui::Button(" ^ "))
				setCurrentDir(currentDir.path().parent_path());
			ImGui::SameLine();

#if _ZE_PLATFORM_WINDOWS
			// Drive selection combo
			char currentDrive = currentDir.path().root_name().string().front();
			char drivePath[5] = " :\\";
			drivePath[0] = currentDrive;
			ImGui::SetNextItemWidth(40.0f);
			if (ImGui::BeginCombo("##dialog_drive", drivePath))
			{
				DWORD driveMask = GetLogicalDrives();
				drivePath[0] = 'A';
				for (; drivePath[0] <= 'Z'; ++drivePath[0])
				{
					if ((driveMask & 1) && std::filesystem::exists(drivePath))
					{
						bool selectedDrive = currentDrive == drivePath[0];
						if (ImGui::Selectable(drivePath, selectedDrive))
							setCurrentDir(drivePath);
						if (selectedDrive)
							ImGui::SetItemDefaultFocus();
					}
					driveMask >>= 1;
				}
				ImGui::EndCombo();
			}
#else
			// Button allowing for return to root dir
			if (ImGui::Button(" / "))
				setCurrentDir("/");
#endif
			ImGui::SameLine();

			// Display path
			ImGui::BeginChild("##dialog_path", { -1.0f, 34.0f }, false, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Dummy({ 0.0f, 0.0f });
			ImGui::Text(currentDir.path().relative_path().string().c_str());
			ImGui::EndChild();

			// Directory structure selection
			ImGui::BeginChild("##dialog_content", { -1.0f, -35.0f }, true);
			auto dirContent = GetDirContent(currentDir, searchType);
			for (U64 i = 0; const auto& entry : dirContent)
			{
				if (ImGui::Selectable(entry.path().filename().string().c_str(), selected == i, ImGuiSelectableFlags_AllowDoubleClick))
				{
					if ((searchType == FileType::Directory) == entry.is_directory())
						selected = i;
					if (ImGui::IsMouseDoubleClicked(0))
					{
						if (entry.is_directory())
						{
							currentDir = entry;
							selected = UINT64_MAX;
						}
					}
				}
				++i;
			}
			ImGui::EndChild();

			// Display current selected entry
			ImGui::BeginChild("##dialog_selected", { -107.0f, 30.0f }, true);
			if (selected != UINT64_MAX)
				ImGui::Text(dirContent.at(selected).path().filename().string().c_str());
			ImGui::EndChild();

			// Entry selection controls
			ImGui::SameLine();
			if (ImGui::Button("Select", { 0.0f, 30.0f }) && selected != UINT64_MAX)
			{
				selectedFile = dirContent.at(selected).path().string();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", { 0.0f, 30.0f }))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		if (selectedFile.size())
			return selectedFile;
		return {};
	}
}