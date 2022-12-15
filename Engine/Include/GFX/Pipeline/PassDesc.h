#pragma once
#include "RendererExecuteData.h"
#include "ResourceID.h"
#include "SyncType.h"

namespace ZE::GFX::Pipeline
{
	// Description of sync required for dependent RenderPass
	struct ExitSync
	{
		SyncType Type;
#if _ZE_RENDER_GRAPH_SINGLE_THREAD
		U64* NextPassFence;
#else
		UA64* NextPassFence;
#endif
	};

	// Info about sync to perform before and after RenderPass
	struct PassSyncDesc
	{
		// Sync required before execution of RenderPass
		SyncType EnterSync = SyncType::None;
#if _ZE_RENDER_GRAPH_SINGLE_THREAD
		U64 EnterFence1 = 0;
		U64 EnterFence2 = 0;
#else
		UA64 EnterFence1 = 0;
		UA64 EnterFence2 = 0;
#endif
		// Syncs required for dependent RenderPasses
		SyncType AllExitSyncs = SyncType::None;
		U8 DependentsCount = 0;
		Ptr<ExitSync> ExitSyncs;
	};

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
	typedef void (*PassCleanCallback)(Device&, void*) noexcept;

	// Descriptor containing params for executing RenderPass
	struct PassDesc
	{
		PassSyncDesc Syncs;
		PassExecuteCallback Execute = nullptr;
		PassData Data;
	};
}