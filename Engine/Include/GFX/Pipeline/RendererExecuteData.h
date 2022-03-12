#pragma once
#include "GFX/Binding/Library.h"
#include "GFX/Resource/CBuffer.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "Data/Tags.h"
#include "FrameBuffer.h"

namespace ZE::GFX::Pipeline
{
	// All structures needed for execution of RenderPass
	struct RendererExecuteData
	{
		// Registry containing all the scene data
		entt::registry Registry;
		// Buffers used for rendering. Initialized by RenderGraph
		FrameBuffer Buffers;
		Binding::Library Bindings;
		Resource::CBuffer SettingsBuffer;
		Resource::CBuffer DynamicBuffer;
		// Global settings of the renderer. Initialized by RenderGraph
		void* SettingsData;
		// Per-frame changing data of the renderer. Initialized by RenderGraph
		void* DynamicData;
		// Initialized by RenderGraph
		Ptr<Resource::PipelineStateGfx> SharedStates;
	};
}