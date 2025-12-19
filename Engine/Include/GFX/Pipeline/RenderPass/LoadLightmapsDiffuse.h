#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/Texture/Pack.h"
#include "Data/CubemapSource.h"

namespace ZE::GFX::Pipeline::RenderPass::LoadLightmapsDiffuse
{
	struct Resources
	{
		RID IrrMap;
	};

	struct ExecuteData
	{
		bool UpdateData = false;
		bool UpdateError = false;
		Data::CubemapSource IrrMapNewSource = {};
		Data::CubemapSource IrrMapSource = {};
		Resource::Texture::Pack IrrMap;
	};

	constexpr bool Evaluate() noexcept { return Settings::IsEnabledIBL(); }

	PassDesc GetDesc(const Data::CubemapSource& irrMapSource) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	void* CopyInitData(void* data) noexcept;
	void FreeInitData(void* data) noexcept;
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, const Data::CubemapSource& irrMapSource);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}