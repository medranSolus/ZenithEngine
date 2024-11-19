#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GUI/ImGuiManager.h"

namespace ZE::GFX::Pipeline::RenderPass::DearImGui
{
	struct Resources
	{
		RID Output;
	};

	struct ExecuteData
	{
		GUI::ImGuiRenderData GuiData;
	};

	constexpr bool Evaluate() noexcept { return Settings::IsEnabledImGui(); }

	PassDesc GetDesc(PixelFormat formatRT) noexcept;
	void Clean(Device& dev, void* data) noexcept;
	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}