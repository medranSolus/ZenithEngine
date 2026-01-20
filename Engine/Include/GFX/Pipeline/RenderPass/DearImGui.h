#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "GUI/ImGuiManager.h"

namespace ZE::GFX::Pipeline::RenderPass::DearImGui
{
	struct Resources
	{
		RID Output;
		RID UI;
	};

	struct ExecuteData
	{
		GUI::ImGuiRenderData GuiData;
		U32 BindingIndex;
		Resource::PipelineStateGfx State;
	};

	constexpr bool Evaluate() noexcept { return Settings::IsEnabledImGui(); }

	PassDesc GetDesc(PixelFormat formatUI, PixelFormat formatRT) noexcept;
	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus);
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatUI, PixelFormat formatRT);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}