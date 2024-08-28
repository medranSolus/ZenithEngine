#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateCompute.h"
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

	struct ExecuteData
	{
		U32 BindingIndexPrefilter;
		U32 BindingIndexAO;
		U32 BindingIndexDenoise;
		Resource::PipelineStateCompute StatePrefilter;
		Resource::PipelineStateCompute StateAO;
		Resource::PipelineStateCompute StateDenoise;
		Resource::Texture::Pack HilbertLUT;
		::XeGTAO::GTAOSettings Settings;
		float SliceCount;
		float StepsPerSlice;
	};

	constexpr bool Evaluate() noexcept { return Settings::GetAOType() == AOType::XeGTAO; }

	void UpdateQualityInfo(ExecuteData& passData) noexcept;
	PassDesc GetDesc() noexcept;
	void Clean(Device& dev, void* data) noexcept;
	void* Initialize(Device& dev, RendererPassBuildData& buildData);
	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData);
}