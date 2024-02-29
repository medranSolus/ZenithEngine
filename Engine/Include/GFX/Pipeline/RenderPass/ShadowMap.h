#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/TransformBuffer.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMap
{
	constexpr U64 BUFFER_SHRINK_STEP = 2;
	constexpr Data::PBRFlags SHADOW_PERMUTATIONS = { Data::MaterialPBR::IsTransparent | Data::MaterialPBR::UseParallaxTex };

	// Indicates that entity is inside view frustum
	struct InsideFrustumSolid { Resource::DynamicBufferAlloc Transform; };
	// Indicates that entity is inside view frustum and is not opaque
	struct InsideFrustumNotSolid {};

#pragma pack(push, 1)
	struct ShaderConstantData
	{
		Float3 LightPos;
		float ParallaxScale;
		U32 Flags;
	};
#pragma pack(pop)

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
	void Setup(Device& dev, RendererBuildData& buildData, ExecuteData& passData,
		PixelFormat formatDS, PixelFormat formatRT, Matrix projection);
	Matrix Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData,
		ExecuteData& data, const Resources& ids, const Float3& lightPos,
		const Float3& lightDir, const Math::BoundingFrustum& frustum);
}