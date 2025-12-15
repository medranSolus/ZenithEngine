#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/Texture/Pack.h"
#include "Data/CubemapSource.h"

namespace ZE::GFX::Pipeline::RenderPass::LoadLightmaps
{
	constexpr U32 BRDF_LUT_SIZE = 256;
	constexpr U32 BRDF_LUT_SAMPLES_COUNT = 512;
	constexpr bool BRDF_LUT_FP16 = true;

	struct Resources
	{
		RID IrrMap;
		RID EnvMap;
		RID BrdfLut;
	};

	struct ExecuteData
	{
		bool UpdateData = false;
		bool UpdateError = false;
		Data::CubemapSource IrrMapNewSource = {};
		Data::CubemapSource IrrMapSource = {};
		Data::CubemapSource EnvMapNewSource = {};
		Data::CubemapSource EnvMapSource = {};
		std::string NewLutSource = "";
		std::string LutSource = "";
		Resource::Texture::Pack IrrMap;
		Resource::Texture::Pack EnvMap;
		Resource::Texture::Pack BrdfLut;
	};

	constexpr bool Evaluate() noexcept { return Settings::IsEnabledSSSR() || Settings::IsEnabledIBL(); }

	PassDesc GetDesc(const std::string& brdfLutSource, const Data::CubemapSource& envMapSource, const Data::CubemapSource& irrMapSource) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	void* CopyInitData(void* data) noexcept;
	void FreeInitData(void* data) noexcept;
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::string& brdfLutSource, const Data::CubemapSource& envMapSource, const Data::CubemapSource& irrMapSource);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}