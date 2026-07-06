#include "GFX/Pipeline/RenderPass/LoadSkybox.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GUI/DearImGui.h"
#include "GUI/DialogWindow.h"

namespace ZE::GFX::Pipeline::RenderPass::LoadSkybox
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

	PassDesc GetDesc(const Data::CubemapSource& source) noexcept
	{
		PassDesc desc{ Base(CorePassType::LoadSkybox) };
		desc.InitData = std::make_unique<InitData>(source);
		desc.Init = Initialize;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.DebugUI = DebugUI;
		return desc;
	}

	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept
	{
		if (!passData.UpdateError && passData.UpdateData && passData.NewSource != passData.SourceData)
		{
			passData.UpdateData = false;
			std::vector<Surface> textures;
			if (passData.NewSource.LoadTextures(textures))
			{
				passData.SourceData = std::move(passData.NewSource);
				Resource::Texture::PackDesc texDesc = {};
				ZE_TEXTURE_SET_NAME(texDesc, "Skybox");
				texDesc.AddTexture(Resource::Texture::Type::Cube, std::move(textures), true);
				ZE_EXPECT_RET_FAILED(passData.SkyTexture, Resource::Texture::Pack::Create(dev, buildData.Assets.GetDisk(), texDesc));
				return UpdateOperation::GpuUploadRequired;
			}
			else
			{
				passData.UpdateError = true;
				passData.NewSource = {};
			}
		}
		return UpdateOperation::NoUpdate;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const InitData& initData) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();
		passData->SourceData = initData.Source;

		Resource::Texture::PackDesc texDesc = {};
		ZE_TEXTURE_SET_NAME(texDesc, "Skybox");

		std::vector<Surface> textures;
		if (!passData->SourceData.LoadTextures(textures))
		{
			Logger::Error("Error loading skybox textures, falling back to generated texture!");
			if (textures.size())
				textures.pop_back();
			for (U8 i = ZE::Utils::SafeCast<U8>(textures.size()); i < 6; i++)
				textures.emplace_back(1, 1);
		}

		texDesc.AddTexture(Resource::Texture::Type::Cube, std::move(textures), true);
		ZE_EXPECT_RET_FAILED(passData->SkyTexture, Resource::Texture::Pack::Create(dev, buildData.Assets.GetDisk(), texDesc));

		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		ZE_CODE_RET_FAILED_EXPECT(renderData.Buffers.RegisterOutsideResource(reinterpret_cast<Resources*>(passData.Resources.get())->Skybox,
			static_cast<ExecuteData*>(passData.ExecData.get())->SkyTexture, 0, FrameResourceType::TextureCube));
		return false;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		ExecuteData& execData = *static_cast<ExecuteData*>(data);
		if (ImGui::CollapsingHeader("Skybox texture"))
		{
			Utils::ShowCubemapDebugUI("Loaded skybox:", execData.SourceData, "Skybox", execData.NewSource, execData.UpdateData, execData.UpdateError);
			ImGui::NewLine();
		}
		if (execData.UpdateError)
		{
			if (GUI::DialogWindow::ShowInfo("Load Error", "Error loading new skybox textures! Falling back to previous skybox."))
				execData.UpdateError = false;
		}
	}
}