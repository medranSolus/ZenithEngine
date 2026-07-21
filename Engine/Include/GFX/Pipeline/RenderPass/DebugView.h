#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::DebugView
{
	enum class Mode : U8
	{
		None,
		Depth = 1,
		Normals = 2,
		Albedo = 3,
		Metalness = 4,
		Roughness = 5,
		Motion = 6,
		Reactive = 7,
		DirectLight = 8,
		SSAO = 9,
		SSR = 10,
		RenderedScene = 11,
		UpscaledScene = 12,
		Outline = 13
	};

	struct Resources
	{
		RID Depth;
		RID Normals;
		RID Albedo;
		RID MaterialParams;
		RID MotionVectors;
		RID ReactiveMask;
		RID DirectLighting;
		RID SSAO;
		RID SSR;
		RID RenderedScene;
		RID UpscaledScene;
		RID Outline;
		RID RenderTarget;
	};

	struct ExecuteData final : public PassExecuteData
	{
		U32 BindingIndex = UINT32_MAX;
		Resource::PipelineStateGfx State;
		Mode ViewMode = Mode::None;
		Mode PrevViewMode = Mode::None;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return Settings::IsEnabledDebugView(); }

	PassDesc GetDesc(PixelFormat outputFormat) noexcept;
	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}