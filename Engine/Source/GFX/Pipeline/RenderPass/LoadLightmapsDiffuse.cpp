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

	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, PassInitData* initData) noexcept
	{
		ZE_ASSERT(initData, "Empty intialization data!");

		return Initialize(dev, buildData, *static_cast<InitData*>(initData));
	}

	PassDesc GetDesc(const Data::CubemapSource& irrMapSource) noexcept
	{
		PassDesc desc{ Base(CorePassType::LoadLightmapsDiffuse) };
		desc.InitData = std::make_unique<InitData>(irrMapSource);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.DebugUI = DebugUI;
		return desc;
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

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const InitData& initData) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();
		passData->IrrMapSource = initData.IrrMapSource;

		Resource::Texture::PackDesc texDesc = {};
		std::vector<Surface> textures;

		ZE_TEXTURE_SET_NAME(texDesc, "Irradiance Map");
		if (!passData->IrrMapSource.LoadTextures(textures))
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
		ZE_CODE_RET_FAILED_EXPECT(renderData.Buffers.RegisterOutsideResource(reinterpret_cast<Resources*>(passData.Resources.get())->IrrMap,
			static_cast<ExecuteData*>(passData.ExecData.get())->IrrMap, 0, FrameResourceType::TextureCube));
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