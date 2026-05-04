#include "GFX/Pipeline/RenderPass/LoadLightmapsDiffuse.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GUI/DearImGui.h"
#include "GUI/DialogWindow.h"

namespace ZE::GFX::Pipeline::RenderPass::LoadLightmapsDiffuse
{
	static Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, PassExecuteData* passData, const std::vector<PixelFormat>& formats) noexcept
	{
		return Update(dev, buildData, *static_cast<ExecuteData*>(passData));
	}

	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) noexcept
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
		desc.CopyInitData = CopyInitData;
		desc.FreeInitData = FreeInitData;
		desc.DebugUI = DebugUI;
		return desc;
	}

	void* CopyInitData(void* data) noexcept
	{
		return new Data::CubemapSource(*reinterpret_cast<Data::CubemapSource*>(data));
	}

	void FreeInitData(void* data) noexcept
	{
		delete reinterpret_cast<Data::CubemapSource*>(data);
	}

	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept
	{
		UpdateOperation status = UpdateOperation::NoUpdate;
		if (!passData.UpdateError && passData.UpdateData)
		{
			if (passData.IrrMapNewSource.Data && passData.IrrMapNewSource != passData.IrrMapSource)
			{
				passData.UpdateData = false;
				std::vector<Surface> textures;
				if (passData.IrrMapNewSource.LoadTextures(textures))
				{
					Resource::Texture::PackDesc texDesc = {};
					ZE_TEXTURE_SET_NAME(texDesc, "Irradiance Map");
					texDesc.AddTexture(Resource::Texture::Type::Cube, std::move(textures));
					ZE_EXPECT_RET_FAILED(passData.IrrMap, Resource::Texture::Pack::Create(dev, buildData.Assets.GetDisk(), texDesc));

					passData.IrrMapSource = std::move(passData.IrrMapNewSource);
					status = UpdateOperation::GpuUploadRequired;
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

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const Data::CubemapSource& irrMapSource) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();
		passData->IrrMapSource = irrMapSource;

		Resource::Texture::PackDesc texDesc = {};
		std::vector<Surface> textures;

		ZE_TEXTURE_SET_NAME(texDesc, "Irradiance Map");
		if (!irrMapSource.LoadTextures(textures))
		{
			Logger::Error("Error loading irradiance map, falling back to generated texture!");
			if (textures.size())
				textures.pop_back();
			for (U8 i = ZE::Utils::SafeCast<U8>(textures.size()); i < 6; i++)
				textures.emplace_back(1, 1);
		}
		texDesc.AddTexture(Resource::Texture::Type::Cube, std::move(textures));
		ZE_EXPECT_RET_FAILED(passData->IrrMap, Resource::Texture::Pack::Create(dev, buildData.Assets.GetDisk(), texDesc));

		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		renderData.Buffers.RegisterOutsideResource(reinterpret_cast<Resources*>(passData.Resources.get())->IrrMap,
			static_cast<ExecuteData*>(passData.ExecData.get())->IrrMap, 0, FrameResourceType::TextureCube);
		return false;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		ExecuteData& execData = *static_cast<ExecuteData*>(data);
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