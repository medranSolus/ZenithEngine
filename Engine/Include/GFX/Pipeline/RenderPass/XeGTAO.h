#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GUI/DearImGui.h"
ZE_WARNING_PUSH
#include "../Include/XeGTAO.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline::RenderPass::XeGTAO
{
	struct Resources
	{
		RID Depth;
		RID Normal;
		RID ViewspaceDepth;
		RID ScratchAO;
		RID DepthEdges;
		RID AO;
	};

	struct ExecuteData final : public PassExecuteData
	{
		U32 BindingIndexPrefilter = UINT32_MAX;
		U32 BindingIndexAO = UINT32_MAX;
		U32 BindingIndexDenoise = UINT32_MAX;
		Resource::PipelineStateCompute StatePrefilter;
		Resource::PipelineStateCompute StateAO;
		Resource::PipelineStateCompute StateDenoise;
		Resource::Texture::Pack HilbertLUT;
		::XeGTAO::GTAOSettings Settings;
		float SliceCount;
		float StepsPerSlice;

		ExecuteData() = default;
		ZE_CLASS_MOVE(ExecuteData);
		virtual ~ExecuteData() = default;
	};

	constexpr bool Evaluate() noexcept { return Settings::AmbientOcclusionType == AOType::XeGTAO; }

	void UpdateQualityInfo(ExecuteData& passData) noexcept;
	PassDesc GetDesc() noexcept;
	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept;
	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept;
	void DebugUI(PassExecuteData* data) noexcept;
}