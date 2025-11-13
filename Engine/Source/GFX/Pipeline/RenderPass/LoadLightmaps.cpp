#include "GFX/Pipeline/RenderPass/LoadLightmaps.h"
#include "GUI/DialogWindow.h"

namespace ZE::GFX::Pipeline::RenderPass::LoadLightmaps
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(initData, "Empty intialization data!");

		std::pair<std::string, Data::CubemapSource> sources = *reinterpret_cast<std::pair<std::string, Data::CubemapSource>*>(initData);
		return Initialize(dev, buildData, sources.first, sources.second);
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
		PassDesc desc{ Base(CorePassType::LoadLightmaps) };
		desc.InitData = new std::pair{ brdfLutSource, envMapSource };
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
		execData->BrdfLut.Free(dev);
		delete execData;
	}

	void* CopyInitData(void* data) noexcept
	{
		return new std::pair(*reinterpret_cast<std::pair<std::string, Data::CubemapSource>*>(data));
	}

	void FreeInitData(void* data) noexcept
	{
		delete reinterpret_cast<std::pair<std::string, Data::CubemapSource>*>(data);
	}

	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData)
	{
		UpdateStatus status = UpdateStatus::NoUpdate;
		if (!passData.UpdateError && passData.UpdateData)
		{
			if (passData.EnvMapNewSource != passData.EnvMapSource)
			{
				passData.UpdateData = false;
				std::vector<Surface> textures;
				if (passData.EnvMapNewSource.LoadTextures(textures))
				{
					passData.EnvMapSource = std::move(passData.EnvMapNewSource);
					Resource::Texture::PackDesc texDesc;
					ZE_TEXTURE_SET_NAME(texDesc, "Environment Map");
					texDesc.AddTexture(Resource::Texture::Type::Cube, std::move(textures));
					passData.EnvMap.Free(dev);
					passData.EnvMap.Init(dev, buildData.Assets.GetDisk(), texDesc);
					status = UpdateStatus::GpuUploadRequired;
				}
				else
				{
					passData.UpdateError = true;
					passData.EnvMapNewSource = {};
				}
			}

			if (passData.NewLutSource != passData.LutSource)
			{
				passData.UpdateData = false;
				std::vector<Surface> textures;
				if (textures.emplace_back().Load(passData.NewLutSource))
				{
					Resource::Texture::PackDesc texDesc;
					ZE_TEXTURE_SET_NAME(texDesc, "BRDF LUT");
					texDesc.AddTexture(Resource::Texture::Type::Tex2D, std::move(textures));
					passData.LutSource = passData.NewLutSource;
					passData.NewLutSource = "";
					passData.BrdfLut.Free(dev);
					passData.BrdfLut.Init(dev, buildData.Assets.GetDisk(), texDesc);
					status = UpdateStatus::GpuUploadRequired;
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

	void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::string& brdfLutSource, const Data::CubemapSource& envMapSource)
	{
		ExecuteData* passData = new ExecuteData;
		passData->EnvMapSource = envMapSource;
		passData->LutSource = brdfLutSource;

		Resource::Texture::PackDesc texDesc;
		ZE_TEXTURE_SET_NAME(texDesc, "BRDF LUT");

		std::vector<Surface> textures;
		if (brdfLutSource.size())
		{
			Surface surf;
			if (surf.Load(brdfLutSource))
				textures.emplace_back(std::move(surf));
			else
			{
				ZE_WARNING("Error loading BRDF LUT from file, falling back to generating default one!");
			}
		}
		if (textures.size() == 0)
			textures.emplace_back(GenerateBrdfLut(BRDF_LUT_SIZE, BRDF_LUT_SAMPLES_COUNT, BRDF_LUT_FP16));

		texDesc.AddTexture(Resource::Texture::Type::Tex2D, std::move(textures));
		passData->BrdfLut.Init(dev, buildData.Assets.GetDisk(), texDesc);

		texDesc.Textures.clear();
		ZE_TEXTURE_SET_NAME(texDesc, "Environment Map");

		if (!envMapSource.LoadTextures(textures))
			throw ZE_RGC_EXCEPT("Error loading environmet map!");

		texDesc.AddTexture(Resource::Texture::Type::Cube, std::move(textures));
		passData->EnvMap.Init(dev, buildData.Assets.GetDisk(), texDesc);

		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		renderData.Buffers.RegisterOutsideResource(passData.Resources.CastConst<Resources>()->BrdfLut,
			passData.ExecData.Cast<ExecuteData>()->BrdfLut, 0, FrameResourceType::Texture2D);
		renderData.Buffers.RegisterOutsideResource(passData.Resources.CastConst<Resources>()->EnvMap,
			passData.ExecData.Cast<ExecuteData>()->EnvMap, 0, FrameResourceType::TextureCube);
		return false;
	}

	void DebugUI(void* data) noexcept
	{
		ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);
		if (ImGui::CollapsingHeader("Lightmaps"))
		{
			ImGui::Text("Loaded environmet map:");
			switch (execData.EnvMapSource.Type)
			{
			case Data::CubemapSourceType::SingleFileCubemap:
			{
				ImGui::BulletText(execData.EnvMapSource.Data[0].c_str());
				break;
			}
			default:
				ZE_ENUM_UNHANDLED();
			case Data::CubemapSourceType::Folder:
			{
				ImGui::BulletText("%s/*%s", execData.EnvMapSource.Data[0].c_str(), execData.EnvMapSource.Data[1].c_str());
				break;
			}
			case Data::CubemapSourceType::CubemapFiles:
			{
				ImGui::BulletText("[+X] %s", execData.EnvMapSource.Data[0].c_str());
				ImGui::BulletText("[-X] %s", execData.EnvMapSource.Data[1].c_str());
				ImGui::BulletText("[+Y] %s", execData.EnvMapSource.Data[2].c_str());
				ImGui::BulletText("[-Y] %s", execData.EnvMapSource.Data[3].c_str());
				ImGui::BulletText("[+Z] %s", execData.EnvMapSource.Data[4].c_str());
				ImGui::BulletText("[-Z] %s", execData.EnvMapSource.Data[5].c_str());
				break;
			}
			}

			ImGui::Text("Load: "); ImGui::SameLine();
			if (const auto selection = GUI::DialogWindow::FileBrowserButton("Directory", "", GUI::DialogWindow::FileType::Image))
			{
				std::filesystem::path path = *selection;
				if (path.has_extension() && path.has_parent_path())
				{
					execData.EnvMapNewSource.InitFolder(std::filesystem::relative(path.parent_path(), std::filesystem::current_path()).string(), path.extension().string());
					execData.UpdateData = true;
				}
				execData.UpdateError = !execData.UpdateData;
			}
			ImGui::SameLine();
			if (const auto selection = GUI::DialogWindow::FileBrowserButton("Single File", "", GUI::DialogWindow::FileType::Image))
			{
				execData.EnvMapNewSource.InitSingleFileCubemap(*selection);
				execData.UpdateData = true;
				execData.UpdateError = !execData.UpdateData;
			}
			ImGui::NewLine();

			ImGui::Text("Loaded BRDF LUT: "); ImGui::SameLine();
			if (execData.LutSource.size())
				ImGui::Text(execData.LutSource.c_str());
			else
				ImGui::Text("Generated %" PRIu32 "x%" PRIu32 ", samples: %" PRIu32 " %s", BRDF_LUT_SIZE, BRDF_LUT_SIZE, BRDF_LUT_SAMPLES_COUNT, BRDF_LUT_FP16 ? "16 bit" : "32 bit");
			ImGui::NewLine();

			if (const auto selection = GUI::DialogWindow::FileBrowserButton("Load BRDF", "", GUI::DialogWindow::FileType::Image))
			{
				execData.NewLutSource = *selection;
				execData.UpdateData = true;
				execData.UpdateError = false;
			}
			ImGui::NewLine();
		}
		if (execData.UpdateError)
		{
			if (GUI::DialogWindow::ShowInfo("Load Error", "Error loading new textures! Falling back to previous lightmap."))
				execData.UpdateError = false;
		}
	}
}