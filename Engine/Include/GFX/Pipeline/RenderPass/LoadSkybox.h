#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/Texture/Pack.h"
#include "Data/CubemapSource.h"

namespace ZE::GFX::Pipeline::RenderPass::LoadSkybox
{
	struct Resources
	{
		RID Skybox;
	};

	struct ExecuteData
	{
		bool UpdateError = false;
		bool UpdateData = false;
		Data::CubemapSource NewSource = {};
		Data::CubemapSource SourceData = {};
		Resource::Texture::Pack SkyTexture;
	};

	PassDesc GetDesc(const Data::CubemapSource& source) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	void* CopyInitData(void* data) noexcept;
	void FreeInitData(void* data) noexcept;
	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, const Data::CubemapSource& source);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
	void DebugUI(void* data) noexcept;
}