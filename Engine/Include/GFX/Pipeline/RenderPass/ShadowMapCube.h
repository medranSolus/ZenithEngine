#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMapCube
{
	constexpr Data::PBRFlags SHADOW_PERMUTATIONS = { Data::MaterialPBR::IsTransparent | Data::MaterialPBR::UseParallaxTex };

	// Indicates that material of the geometry can be processed in depth pre-pass
	struct Solid { Resource::DynamicBufferAlloc Transform; };
	// Indicates that material of the geometry cannot be processed in depth pre-pass
	struct Transparent {};

	struct Resources
	{
		RID RenderTarget;
		RID Depth;
	};

	struct ExecuteData
	{
		U32 BindingIndex;
		Resource::PipelineStateGfx StateDepth;
		Ptr<Resource::PipelineStateGfx> StatesSolid;
		Ptr<Resource::PipelineStateGfx> StatesTransparent;
		Float4x4 Projection;
	};

	void Clean(Device& dev, ExecuteData& data) noexcept;
	void Initialize(Device& dev, RendererPassBuildData& buildData,
		ExecuteData& passData, PixelFormat formatDS, PixelFormat formatRT);
	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData,
		ExecuteData& data, const Resources& ids, const Float3& lightPos, float lightVolume);
}