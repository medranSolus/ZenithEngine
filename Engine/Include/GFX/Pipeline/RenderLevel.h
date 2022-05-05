#pragma once
#include "RendererExecuteData.h"
#include "ResourceID.h"
#include "SyncType.h"

namespace ZE::GFX::Pipeline
{
	// Data needed for executing RenderPass
	struct PassData
	{
		// Resources used by RenderPass, appear in order they've been added during RenderNode construction,
		// in respect to their usage: first input, then inner, lastly output resources (without already present input resources)
		Ptr<const RID> Buffers = nullptr;
		// Optional data used by RenderPass
		void* OptData = nullptr;
	};

	// Callback for pass execution
	typedef void (*PassExecuteCallback)(Device&, CommandList&, RendererExecuteData&, PassData&);
	// Callback for cleaning optional pass data
	typedef void (*PassCleanCallback)(void*);

	// Descriptor containing params for executing RenderPass
	struct PassDesc
	{
		PassExecuteCallback Execute = nullptr;
		PassData Data;
	};

	// Data about render passes allowed for execution betweem queue sync points
	struct RenderLevel
	{
		SyncType EnterSync = SyncType::None;
		U16 LevelCount;
		// [0]:Gfx, [1]:Compute bundles
		Ptr<std::pair<Ptr<PassDesc>, U16>> Bundles[2];
		SyncType ExitSync = SyncType::None;
	};
}