#pragma once
#include "GFX/Pipeline/PassDesc.h"
#include "GFX/Pipeline/RendererBuildData.h"
#include "GFX/Pipeline/WorldInfo.h"

namespace ZE::GFX::Pipeline::RenderPass::Lambertian
{
	constexpr U32 SINGLE_BUFFER_SIZE = 64 * 1024;
	constexpr U64 BUFFER_SHRINK_STEP = 2;

	struct Resources
	{
		RID DepthStencil;
		RID Color;
		RID Normal;
		RID Specular;
	};

	struct Data
	{
		WorldInfo& World;
		U32 BindingIndex;
		Resource::PipelineStateGfx StateDepth;
		Resource::PipelineStateGfx StateNormal;
		std::vector<Resource::CBuffer> TransformBuffers;
	};

	struct ModelTransform
	{
		Matrix Model;
		Matrix ModelViewProjection;
	};

	// After changin content of the transform buffer, set size of transform array in
	// Engine/Shader/VS/CB/Transform.hlsli to SINGLE_BUFFER_SIZE / sizeof(TransformCBuffer)
	struct TransformCBuffer
	{
		Float3 CameraPos;
		float padding;
		ModelTransform Transforms[511];
	};

	inline void Clean(void* data) { delete reinterpret_cast<Data*>(data); }

	Data* Setup(Device& dev, RendererBuildData& buildData, WorldInfo& worldData, PixelFormat formatDS,
		PixelFormat formatColor, PixelFormat formatNormal, PixelFormat formatSpecular);
	void Execute(RendererExecuteData& renderData, PassData& passData);
}