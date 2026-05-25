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

	struct InitData final : public PassInitData
	{
		Data::CubemapSource Source = {};

		InitData() = default;
		InitData(const Data::CubemapSource& src) noexcept : Source(src) {}
		ZE_CLASS_DEFAULT(InitData);
		virtual ~InitData() = default;

		std::unique_ptr<PassInitData> Clone() const noexcept override { return std::make_unique<InitData>(*this); }
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
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const InitData& initData) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}