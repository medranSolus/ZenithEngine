#include "GFX/Pipeline/RenderPass/LoadLightmapsDiffuse.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GUI/DialogWindow.h"

namespace ZE::GFX::Pipeline::RenderPass::LoadLightmapsDiffuse
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(initData, "Empty intialization data!");

		const Data::CubemapSource& sources = *reinterpret_cast<Data::CubemapSource*>(initData);
		return Initialize(dev, buildData, sources);
	}

	PassDesc GetDesc(const Data::CubemapSource& irrMapSource) noexcept
	{
		PassDesc desc{ Base(CorePassType::LoadLightmapsDiffuse) };
		desc.InitData = new Data::CubemapSource{ irrMapSource };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
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
		execData->IrrMap.Free(dev);
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
		UpdateStatus status = UpdateStatus::NoUpdate;
		if (!passData.UpdateError && passData.UpdateData)
		{
			if (passData.IrrMapNewSource.Data && passData.IrrMapNewSource != passData.IrrMapSource)
			{
				passData.UpdateData = false;
				std::vector<Surface> textures;
				if (passData.IrrMapNewSource.LoadTextures(textures))
				{
					passData.IrrMapSource = std::move(passData.IrrMapNewSource);
					Resource::Texture::PackDesc texDesc;
					ZE_TEXTURE_SET_NAME(texDesc, "Irradiance Map");
					texDesc.AddTexture(Resource::Texture::Type::Cube, std::move(textures));

					buildData.SyncStatus.SyncMain(dev);
					buildData.SyncStatus.SyncCompute(dev);
					passData.IrrMap.Free(dev);
					passData.IrrMap.Init(dev, buildData.Assets.GetDisk(), texDesc);
					status = UpdateStatus::GpuUploadRequired;
				}
				else
				{
					passData.UpdateError = true;
					passData.IrrMapNewSource = {};
				}
			}
		}
		return status;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, const Data::CubemapSource& irrMapSource)
	{
		ExecuteData* passData = new ExecuteData;
		passData->IrrMapSource = irrMapSource;

		Resource::Texture::PackDesc texDesc;
		std::vector<Surface> textures;

		ZE_TEXTURE_SET_NAME(texDesc, "Irradiance Map");
		if (!irrMapSource.LoadTextures(textures))
			throw ZE_RGC_EXCEPT("Error loading irradiance map!");
		texDesc.AddTexture(Resource::Texture::Type::Cube, std::move(textures));
		passData->IrrMap.Init(dev, buildData.Assets.GetDisk(), texDesc);

		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		renderData.Buffers.RegisterOutsideResource(passData.Resources.CastConst<Resources>()->IrrMap,
			passData.ExecData.Cast<ExecuteData>()->IrrMap, 0, FrameResourceType::TextureCube);
		return false;
	}

	void DebugUI(void* data) noexcept
	{
		ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);
		if (ImGui::CollapsingHeader("Diffuse Lightmaps"))
		{
			Utils::ShowCubemapDebugUI("Loaded irradiance map:", execData.IrrMapSource, "", execData.IrrMapNewSource, execData.UpdateData, execData.UpdateError);
			ImGui::NewLine();
		}
		if (execData.UpdateError)
		{
			if (GUI::DialogWindow::ShowInfo("Load Error", "Error loading irradiance map textures! Falling back to previous lightmap."))
				execData.UpdateError = false;
		}
	}
}