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
		UA64* NextPassFence;
	};

	// Info about sync to perform before and after RenderPass
	struct PassSyncDesc
	{
		// Sync required before execution of RenderPass
		SyncType EnterSync = SyncType::None;
		UA64 EnterFence1 = 0;
		UA64 EnterFence2 = 0;
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
	typedef void (*PassExecuteCallback)(RendererExecuteData&, PassData&);
	// Callback for cleaning optional pass data
	typedef void (*PassCleanCallback)(void*);

	// Descriptor containing params for executing RenderPass
	struct PassDesc
	{
		PassSyncDesc Syncs;
		PassExecuteCallback Execute = nullptr;
		PassData Data;
	};

	// Descriptor containing commands for static passes in single level
	struct PassDescStatic
	{
		PassSyncDesc Syncs;
		U32 CommandsCount = 0;
		Ptr<CommandList> Commands;
	};
}