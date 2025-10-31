#include "GFX/Pipeline/RenderPass/LoadSkybox.h"
#include "GUI/DialogWindow.h"

namespace ZE::GFX::Pipeline::RenderPass::LoadSkybox
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(initData, "Empty intialization data!");

		return Initialize(dev, buildData, *reinterpret_cast<Data::SkyboxSource*>(initData));
	}

	static bool LoadTextures(const Data::SkyboxSource& source, std::vector<Surface>& textures) noexcept
	{
		if (source.Data == nullptr)
			return false;

		bool result = true;
		switch (source.Type)
		{
		case Data::SkyboxType::SingleFileCubemap:
		{
			result = textures.emplace_back().Load(source.Data[0]);
			break;
		}
		default:
			ZE_ENUM_UNHANDLED();
		case Data::SkyboxType::Folder:
		{
			textures.reserve(6);
			result = textures.emplace_back().Load(source.Data[0] + "/px" + source.Data[1]); // Right
			if (result)
				result &= textures.emplace_back().Load(source.Data[0] + "/nx" + source.Data[1]); // Left
			if (result)
				result &= textures.emplace_back().Load(source.Data[0] + "/py" + source.Data[1]); // Up
			if (result)
				result &= textures.emplace_back().Load(source.Data[0] + "/ny" + source.Data[1]); // Down
			if (result)
				result &= textures.emplace_back().Load(source.Data[0] + "/pz" + source.Data[1]); // Front
			if (result)
				result &= textures.emplace_back().Load(source.Data[0] + "/nz" + source.Data[1]); // Back
			break;
		}
		case Data::SkyboxType::CubemapFiles:
		{
			textures.reserve(6);
			result = textures.emplace_back().Load(source.Data[0]); // Right
			if (result)
				result &= textures.emplace_back().Load(source.Data[1]); // Left
			if (result)
				result &= textures.emplace_back().Load(source.Data[2]); // Up
			if (result)
				result &= textures.emplace_back().Load(source.Data[3]); // Down
			if (result)
				result &= textures.emplace_back().Load(source.Data[4]); // Front
			if (result)
				result &= textures.emplace_back().Load(source.Data[5]); // Back
			break;
		}
		}
		return result;
	}

	PassDesc GetDesc(const Data::SkyboxSource& source) noexcept
	{
		PassDesc desc{ Base(CorePassType::LoadSkybox) };
		desc.InitData = new Data::SkyboxSource{ source };
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
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->SkyTexture.Free(dev);
		delete execData;
	}

	void* CopyInitData(void* data) noexcept
	{
		return new Data::SkyboxSource(*reinterpret_cast<Data::SkyboxSource*>(data));
	}

	void FreeInitData(void* data) noexcept
	{
		delete reinterpret_cast<Data::SkyboxSource*>(data);
	}

	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData)
	{
		if (!passData.UpdateError && passData.UpdateData && passData.NewSource != passData.SourceData)
		{
			passData.UpdateData = false;
			std::vector<Surface> textures;
			if (LoadTextures(passData.NewSource, textures))
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

	void* Initialize(Device& dev, RendererPassBuildData& buildData, const Data::SkyboxSource& source)
	{
		ExecuteData* passData = new ExecuteData;
		passData->SourceData = source;

		Resource::Texture::PackDesc texDesc;
		ZE_TEXTURE_SET_NAME(texDesc, "Skybox");

		std::vector<Surface> textures;
		if (!LoadTextures(source, textures))
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
			case Data::SkyboxType::SingleFileCubemap:
			{
				ImGui::BulletText(execData.SourceData.Data[0].c_str());
				break;
			}
			default:
				ZE_ENUM_UNHANDLED();
			case Data::SkyboxType::Folder:
			{
				ImGui::BulletText("%s/*%s", execData.SourceData.Data[0].c_str(), execData.SourceData.Data[1].c_str());
				break;
			}
			case Data::SkyboxType::CubemapFiles:
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