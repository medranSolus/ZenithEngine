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

	struct ExecuteData final : public PassExecuteData
	{
		bool UpdateError = false;
		bool UpdateData = false;
		Data::CubemapSource NewSource = {};
		Data::CubemapSource SourceData = {};
		Resource::Texture::Pack SkyTexture;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	PassDesc GetDesc(const Data::CubemapSource& source) noexcept;
	void* CopyInitData(void* data) noexcept;
	void FreeInitData(void* data) noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	Expected<std::unique_ptr<ExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData, const Data::CubemapSource& source) noexcept;
	Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}