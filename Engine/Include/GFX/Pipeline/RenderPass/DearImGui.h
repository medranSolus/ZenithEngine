#pragma once
#include "GFX/External/ImGuiBackendData.h"
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::DearImGui
{
	struct Resources
	{
		RID Output;
		RID UI;
	};

	struct ExecuteData final : public PassExecuteData
	{
		External::ImGuiBackendData GuiData;
		U32 BindingIndex = UINT32_MAX;
		Resource::PipelineStateGfx State;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return Settings::IsEnabledImGui(); }

	PassDesc GetDesc(PixelFormat formatUI, PixelFormat formatRT) noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatUI, PixelFormat formatRT) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
}