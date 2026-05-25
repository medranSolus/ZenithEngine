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

	struct InitData final : public PassInitData
	{
		Data::CubemapSource IrrMapSource = {};

		InitData() = default;
		InitData(const Data::CubemapSource& src) noexcept : IrrMapSource(src) {}
		ZE_CLASS_DEFAULT(InitData);
		virtual ~InitData() = default;

		std::unique_ptr<PassInitData> Clone() const noexcept override { return std::make_unique<InitData>(*this); }
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
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const InitData& initData) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}