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

	struct ExecuteData final : public PassExecuteData
	{
		GUI::ImGuiRenderData GuiData = {};
		U32 BindingIndex = UINT32_MAX;
		Resource::PipelineStateGfx State;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData();
	};

	constexpr bool Evaluate() noexcept { return Settings::IsEnabledImGui(); }

	PassDesc GetDesc(PixelFormat formatUI, PixelFormat formatRT) noexcept;
	Expected<std::unique_ptr<ExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatUI, PixelFormat formatRT) noexcept;
	Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
}