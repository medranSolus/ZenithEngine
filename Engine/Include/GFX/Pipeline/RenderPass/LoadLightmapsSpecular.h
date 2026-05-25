#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/Texture/Pack.h"
#include "Data/CubemapSource.h"

namespace ZE::GFX::Pipeline::RenderPass::LoadLightmapsSpecular
{
	constexpr U32 BRDF_LUT_SIZE = 256;
	constexpr U32 BRDF_LUT_SAMPLES_COUNT = 512;
	constexpr bool BRDF_LUT_FP16 = true;

	struct Resources
	{
		RID EnvMap;
		RID BrdfLut;
	};

	struct InitData final : public PassInitData
	{
		std::string LutSource = "";
		Data::CubemapSource EnvMapSource = {};

		InitData() = default;
		InitData(const std::string& lut, const Data::CubemapSource& env) noexcept : LutSource(lut), EnvMapSource(env) {}
		ZE_CLASS_DEFAULT(InitData);
		virtual ~InitData() = default;

		std::unique_ptr<PassInitData> Clone() const noexcept override { return std::make_unique<InitData>(*this); }
	};

	struct ExecuteData final : public PassExecuteData
	{
		bool UpdateData = false;
		bool UpdateError = false;
		Data::CubemapSource EnvMapNewSource = {};
		Data::CubemapSource EnvMapSource = {};
		std::string NewLutSource = "";
		std::string LutSource = "";
		Resource::Texture::Pack EnvMap;
		Resource::Texture::Pack BrdfLut;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return Settings::IsEnabledSSSR() || Settings::IsEnabledIBL(); }

	PassDesc GetDesc(const std::string& brdfLutSource, const Data::CubemapSource& envMapSource) noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const InitData& initData) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}