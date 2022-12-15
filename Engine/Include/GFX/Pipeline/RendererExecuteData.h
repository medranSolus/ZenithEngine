#pragma once
#include "GFX/Binding/Library.h"
#include "GFX/Resource/CBuffer.h"
#include "GFX/Resource/DynamicCBuffer.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "GFX/ChainPool.h"
#include "Data/Tags.h"
#include "FrameBuffer.h"

namespace ZE::GFX::Pipeline
{
	// All structures needed for execution of RenderPass
	struct RendererExecuteData
	{
		// Alloc inside DynamicBuffers corresponding to DynamicData of the renderer
		static constexpr Resource::DynamicBufferAlloc RENDERER_DYNAMIC_BUFFER = { 0, 0 };

		// Registry containing all the scene data
		Data::Storage Registry;
		// Registry containing material and geometry data
		Data::Storage Resources;
		// Buffers used for rendering. Initialized by RenderGraph
		FrameBuffer Buffers;
		Binding::Library Bindings;
		Resource::CBuffer SettingsBuffer;
		ChainPool<Resource::DynamicCBuffer> DynamicBuffers;
		// Global settings of the renderer. Initialized by RenderGraph
		void* SettingsData;
		// Per-frame changing data of the renderer. Initialized by RenderGraph
		void* DynamicData;
		// Renderer issuing current execution. Initialized by RenderGraph
		void* Renderer;
		// Initialized by RenderGraph
		Ptr<Resource::PipelineStateGfx> SharedStates;
		U64 SharedStatesCount = 0;

		void BindRendererDynamicData(CommandList& cl, Binding::Context& bindCtx) const noexcept { DynamicBuffers.Get().Bind(cl, bindCtx, RENDERER_DYNAMIC_BUFFER); }
	};
}