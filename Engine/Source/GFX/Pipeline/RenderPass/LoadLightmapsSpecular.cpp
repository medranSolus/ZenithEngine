#include "GFX/Pipeline/RenderPass/LoadLightmapsSpecular.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GUI/DearImGui.h"
#include "GUI/DialogWindow.h"

namespace ZE::GFX::Pipeline::RenderPass::LoadLightmapsSpecular
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

	static Surface GenerateBrdfLut(U32 size, U32 samples, bool fp16) noexcept
	{
		Surface lut(size, size, fp16 ? PixelFormat::R16G16_Float : PixelFormat::R32G32_Float);

		const float step = 1.0f / static_cast<float>(size);
		const U32 rowSize = lut.GetRowByteSize();
		U8* image = lut.GetBuffer();
		for (U32 y = 0; y < size; ++y)
		{
			for (U32 x = 0; x < size; ++x)
			{
				const float NdotV = (static_cast<float>(x) + 0.5f) * step;
				const float roughness = (static_cast<float>(y) + 0.5f) * step;
				Float2 sample = Math::Light::IntegrateBRDF(NdotV, roughness, samples);

				if (fp16)
				{
					U32 packedValue = Math::FP16::EncodeFloat16Fast(sample.x);
					packedValue |= static_cast<U32>(Math::FP16::EncodeFloat16Fast(sample.y)) << 16;
					reinterpret_cast<U32*>(image)[x] = packedValue;
				}
				else
					reinterpret_cast<Float2*>(image)[x] = sample;
			}
			image += rowSize;
		}
		return lut;
	}

	PassDesc GetDesc(const std::string& brdfLutSource, const Data::CubemapSource& envMapSource) noexcept
	{
		PassDesc desc{ Base(CorePassType::LoadLightmapsSpecular) };
		desc.InitData = std::make_unique<InitData>(brdfLutSource, envMapSource);
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
			if (passData.EnvMapNewSource.Data && passData.EnvMapNewSource != passData.EnvMapSource)
			{
				passData.UpdateData = false;
				std::vector<Surface> textures;
				if (passData.EnvMapNewSource.LoadTextures(textures))
				{
					Resource::Texture::PackDesc texDesc = {};
					ZE_TEXTURE_SET_NAME(texDesc, "Environment Map");
					texDesc.AddTexture(Resource::Texture::Type::Cube, std::move(textures));
					ZE_EXPECT_RET_FAILED(passData.EnvMap, Resource::Texture::Pack::Create(dev, buildData.Assets.GetDisk(), texDesc));

					passData.EnvMapSource = std::move(passData.EnvMapNewSource);
					status = UpdateOperation::GpuUploadRequired;
				}
				else
				{
					passData.UpdateError = true;
					passData.EnvMapNewSource = {};
				}
			}

			if (!passData.NewLutSource.empty() && passData.NewLutSource != passData.LutSource)
			{
				passData.UpdateData = false;
				std::vector<Surface> textures;
				if (textures.emplace_back().Load(passData.NewLutSource))
				{
					Resource::Texture::PackDesc texDesc = {};
					ZE_TEXTURE_SET_NAME(texDesc, "BRDF LUT");
					texDesc.AddTexture(Resource::Texture::Type::Tex2D, std::move(textures));
					ZE_EXPECT_RET_FAILED(passData.BrdfLut, Resource::Texture::Pack::Create(dev, buildData.Assets.GetDisk(), texDesc));

					passData.LutSource = std::move(passData.NewLutSource);
					status = UpdateOperation::GpuUploadRequired;
				}
				else
				{
					passData.UpdateError = true;
					passData.NewLutSource = "";
				}
			}
		}
		return status;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const InitData& initData) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();
		passData->EnvMapSource = initData.EnvMapSource;
		passData->LutSource = initData.LutSource;
		Resource::Texture::PackDesc texDesc = {};
		std::vector<Surface> textures;

		ZE_TEXTURE_SET_NAME(texDesc, "Environment Map");
		if (!passData->EnvMapSource.LoadTextures(textures))
		{
			Logger::Error("Error loading environmet map, falling back to generated texture!");
			if (textures.size())
				textures.pop_back();
			for (U8 i = ZE::Utils::SafeCast<U8>(textures.size()); i < 6; i++)
				textures.emplace_back(1, 1);
		}
		texDesc.AddTexture(Resource::Texture::Type::Cube, std::move(textures));
		ZE_EXPECT_RET_FAILED(passData->EnvMap, Resource::Texture::Pack::Create(dev, buildData.Assets.GetDisk(), texDesc));

		texDesc.Textures.clear();
		ZE_TEXTURE_SET_NAME(texDesc, "BRDF LUT");
		if (passData->LutSource.size())
		{
			Surface surf;
			if (surf.Load(passData->LutSource))
				textures.emplace_back(std::move(surf));
			else
				Logger::Warning("Error loading BRDF LUT from file, falling back to generating default one!");
		}
		if (textures.size() == 0)
			textures.emplace_back(GenerateBrdfLut(BRDF_LUT_SIZE, BRDF_LUT_SAMPLES_COUNT, BRDF_LUT_FP16));
		texDesc.AddTexture(Resource::Texture::Type::Tex2D, std::move(textures));
		ZE_EXPECT_RET_FAILED(passData->BrdfLut, Resource::Texture::Pack::Create(dev, buildData.Assets.GetDisk(), texDesc));

		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		ZE_CODE_RET_FAILED_EXPECT(renderData.Buffers.RegisterOutsideResource(reinterpret_cast<Resources*>(passData.Resources.get())->EnvMap,
			static_cast<ExecuteData*>(passData.ExecData.get())->EnvMap, 0, FrameResourceType::TextureCube));
		ZE_CODE_RET_FAILED_EXPECT(renderData.Buffers.RegisterOutsideResource(reinterpret_cast<Resources*>(passData.Resources.get())->BrdfLut,
			static_cast<ExecuteData*>(passData.ExecData.get())->BrdfLut, 0, FrameResourceType::Texture2D));
		return false;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		ExecuteData& execData = *static_cast<ExecuteData*>(data);
		if (ImGui::CollapsingHeader("Specular Lightmaps"))
		{
			Utils::ShowCubemapDebugUI("Loaded environmet map:", execData.EnvMapSource, "", execData.EnvMapNewSource, execData.UpdateData, execData.UpdateError);

			ImGui::Text("Loaded BRDF LUT: "); ImGui::SameLine();
			if (execData.LutSource.size())
				ImGui::Text(execData.LutSource.c_str());
			else
				ImGui::Text("Generated %" PRIu32 "x%" PRIu32 ", samples: %" PRIu32 " %s", BRDF_LUT_SIZE, BRDF_LUT_SIZE, BRDF_LUT_SAMPLES_COUNT, BRDF_LUT_FP16 ? "16 bit" : "32 bit");

			if (const auto selectionExp = GUI::DialogWindow::FileBrowserButton("Load BRDF", "", GUI::DialogWindow::FileType::Image))
			{
				if (selectionExp)
				{
					if (*selectionExp)
					{
						execData.NewLutSource = **selectionExp;
						execData.UpdateData = true;
						execData.UpdateError = false;
					}
				}
				else
				{
					ZE_CODE_ERROR(selectionExp.error(), "Error loading BRDF!");
					execData.UpdateError = true;
				}
			}
			ImGui::NewLine();
		}
		if (execData.UpdateError)
		{
			if (GUI::DialogWindow::ShowInfo("Load Error", "Error loading new specular textures! Falling back to previous lightmap."))
				execData.UpdateError = false;
		}
	}
}