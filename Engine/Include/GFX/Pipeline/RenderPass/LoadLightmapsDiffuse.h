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

	struct ExecuteData final : public PassExecuteData
	{
		bool UpdateData = false;
		bool UpdateError = false;
		Data::CubemapSource IrrMapNewSource = {};
		Data::CubemapSource IrrMapSource = {};
		Resource::Texture::Pack IrrMap;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return Settings::IsEnabledIBL(); }

	PassDesc GetDesc(const Data::CubemapSource& irrMapSource) noexcept;
	void* CopyInitData(void* data) noexcept;
	void FreeInitData(void* data) noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	Expected<std::unique_ptr<ExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData, const Data::CubemapSource& irrMapSource) noexcept;
	Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}