#pragma once
#include "GFX/Binding/Library.h"
#include "GFX/Resource/CBuffer.h"
#include "GFX/Resource/DynamicCBuffer.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "Data/AssetsStreamer.h"
#include "Data/Tags.h"
#include "FrameBuffer.h"

namespace ZE::GFX::Pipeline
{
	// All structures needed for execution of RenderPass
	struct RendererExecuteData
	{
		// Alloc inside DynamicBuffers corresponding to DynamicData of the renderer
		static constexpr Resource::DynamicBufferAlloc RENDERER_DYNAMIC_BUFFER = { 0, 0 };

		// Registry containing material and geometry data
		Data::AssetsStreamer Assets;
		// Buffers used for rendering. Initialized by RenderGraph
		FrameBuffer Buffers;
		Binding::Library Bindings;
		Resource::CBuffer SettingsBuffer;
		// Current dynamic constant buffer used for uploading data to GPU. Initialized by RenderGraph
		Ptr<Resource::DynamicCBuffer> DynamicBuffer;
		// Global settings of the renderer. Initialized by RenderGraph
		void* SettingsData;
		// Per-frame changing data of the renderer. Initialized by RenderGraph
		void* DynamicData;
		// Renderer issuing current execution. Initialized by RenderGraph
		void* Renderer;
		// Initialized by RenderGraph
		Ptr<Resource::PipelineStateGfx> SharedStates;
		U64 SharedStatesCount = 0;

		void BindRendererDynamicData(CommandList& cl, Binding::Context& bindCtx) const noexcept { DynamicBuffer->Bind(cl, bindCtx, RENDERER_DYNAMIC_BUFFER); }
	};
}