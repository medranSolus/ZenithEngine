#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GUI/DialogWindow.h"

namespace ZE::GFX::Pipeline::RenderPass::Utils
{
	void ShowCubemapDebugUI(const char* title, const Data::CubemapSource& source, const char* newSourceDir, Data::CubemapSource& newSource, bool& updateData, bool& updateError) noexcept
	{
		ImGui::Text(title);
		switch (source.Type)
		{
		case Data::CubemapSourceType::SingleFileCubemap:
		{
			ImGui::BulletText(source.Data[0].c_str());
			break;
		}
		default:
			ZE_ENUM_UNHANDLED();
		case Data::CubemapSourceType::Folder:
		{
			ImGui::BulletText("%s/*%s", source.Data[0].c_str(), source.Data[1].c_str());
			break;
		}
		case Data::CubemapSourceType::CubemapFiles:
		{
			ImGui::BulletText("[+X] %s", source.Data[0].c_str());
			ImGui::BulletText("[-X] %s", source.Data[1].c_str());
			ImGui::BulletText("[+Y] %s", source.Data[2].c_str());
			ImGui::BulletText("[-Y] %s", source.Data[3].c_str());
			ImGui::BulletText("[+Z] %s", source.Data[4].c_str());
			ImGui::BulletText("[-Z] %s", source.Data[5].c_str());
			break;
		}
		}

		ImGui::PushID(title);
		ImGui::Text("Load: "); ImGui::SameLine();
		if (const auto selection = GUI::DialogWindow::FileBrowserButton("Directory", newSourceDir, GUI::DialogWindow::FileType::Image))
		{
			std::filesystem::path path = *selection;
			if (path.has_extension() && path.has_parent_path())
			{
				newSource.InitFolder(std::filesystem::relative(path.parent_path(), std::filesystem::current_path()).string(), path.extension().string());
				updateData = true;
			}
			updateError = !updateData;
		}
		ImGui::SameLine();
		if (const auto selection = GUI::DialogWindow::FileBrowserButton("Single File", newSourceDir, GUI::DialogWindow::FileType::Image))
		{
			newSource.InitSingleFileCubemap(*selection);
			updateData = true;
			updateError = false;
		}
		ImGui::PopID();
	}
}