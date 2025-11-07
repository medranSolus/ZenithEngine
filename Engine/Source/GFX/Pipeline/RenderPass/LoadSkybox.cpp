#include "GFX/Pipeline/RenderPass/LoadSkybox.h"
#include "GUI/DialogWindow.h"

namespace ZE::GFX::Pipeline::RenderPass::LoadSkybox
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(initData, "Empty intialization data!");

		return Initialize(dev, buildData, *reinterpret_cast<Data::CubemapSource*>(initData));
	}

	PassDesc GetDesc(const Data::CubemapSource& source) noexcept
	{
		PassDesc desc{ Base(CorePassType::LoadSkybox) };
		desc.InitData = new Data::CubemapSource{ source };
		desc.Init = Initialize;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		desc.CopyInitData = CopyInitData;
		desc.FreeInitData = FreeInitData;
		desc.DebugUI = DebugUI;
		return desc;
	}

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		syncStatus.SyncMain(dev);
		syncStatus.SyncCompute(dev);
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->SkyTexture.Free(dev);
		delete execData;
	}

	void* CopyInitData(void* data) noexcept
	{
		return new Data::CubemapSource(*reinterpret_cast<Data::CubemapSource*>(data));
	}

	void FreeInitData(void* data) noexcept
	{
		delete reinterpret_cast<Data::CubemapSource*>(data);
	}

	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData)
	{
		if (!passData.UpdateError && passData.UpdateData && passData.NewSource != passData.SourceData)
		{
			passData.UpdateData = false;
			std::vector<Surface> textures;
			if (passData.NewSource.LoadTextures(textures))
			{
				passData.SourceData = std::move(passData.NewSource);
				Resource::Texture::PackDesc texDesc;
				ZE_TEXTURE_SET_NAME(texDesc, "Skybox");
				texDesc.AddTexture(Resource::Texture::Type::Cube, std::move(textures));
				passData.SkyTexture.Free(dev);
				passData.SkyTexture.Init(dev, buildData.Assets.GetDisk(), texDesc);
				return UpdateStatus::GpuUploadRequired;
			}
			else
			{
				passData.UpdateError = true;
				passData.NewSource = {};
			}
		}
		return UpdateStatus::NoUpdate;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, const Data::CubemapSource& source)
	{
		ExecuteData* passData = new ExecuteData;
		passData->SourceData = source;

		Resource::Texture::PackDesc texDesc;
		ZE_TEXTURE_SET_NAME(texDesc, "Skybox");

		std::vector<Surface> textures;
		if (!source.LoadTextures(textures))
			throw ZE_RGC_EXCEPT("Error loading cubemap!");

		texDesc.AddTexture(Resource::Texture::Type::Cube, std::move(textures));
		passData->SkyTexture.Init(dev, buildData.Assets.GetDisk(), texDesc);

		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		renderData.Buffers.RegisterOutsideResource(passData.Resources.CastConst<Resources>()->Skybox,
			passData.ExecData.Cast<ExecuteData>()->SkyTexture, 0, FrameResourceType::TextureCube);
		return false;
	}

	void DebugUI(void* data) noexcept
	{
		ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);
		if (ImGui::CollapsingHeader("Skybox texture"))
		{
			ImGui::Text("Loaded skybox:");
			switch (execData.SourceData.Type)
			{
			case Data::CubemapSourceType::SingleFileCubemap:
			{
				ImGui::BulletText(execData.SourceData.Data[0].c_str());
				break;
			}
			default:
				ZE_ENUM_UNHANDLED();
			case Data::CubemapSourceType::Folder:
			{
				ImGui::BulletText("%s/*%s", execData.SourceData.Data[0].c_str(), execData.SourceData.Data[1].c_str());
				break;
			}
			case Data::CubemapSourceType::CubemapFiles:
			{
				ImGui::BulletText("[+X] %s", execData.SourceData.Data[0].c_str());
				ImGui::BulletText("[-X] %s", execData.SourceData.Data[1].c_str());
				ImGui::BulletText("[+Y] %s", execData.SourceData.Data[2].c_str());
				ImGui::BulletText("[-Y] %s", execData.SourceData.Data[3].c_str());
				ImGui::BulletText("[+Z] %s", execData.SourceData.Data[4].c_str());
				ImGui::BulletText("[-Z] %s", execData.SourceData.Data[5].c_str());
				break;
			}
			}

			ImGui::Text("Load: "); ImGui::SameLine();
			if (const auto selection = GUI::DialogWindow::FileBrowserButton("Directory", "Skybox", GUI::DialogWindow::FileType::Image))
			{
				std::filesystem::path path = *selection;
				if (path.has_extension() && path.has_parent_path())
				{
					execData.NewSource.InitFolder(std::filesystem::relative(path.parent_path(), std::filesystem::current_path()).string(), path.extension().string());
					execData.UpdateData = true;
				}
				execData.UpdateError = !execData.UpdateData;
			}
			ImGui::NewLine();
		}
		if (execData.UpdateError)
		{
			if (GUI::DialogWindow::ShowInfo("Load Error", "Error loading new skybox textures! Falling back to previous skybox."))
				execData.UpdateError = false;
		}
	}
}