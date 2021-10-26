#pragma once
#include "GFX/CommandList.h"
#include "SyncType.h"

namespace ZE::GFX::Pipeline
{
	// Callback for pass execution
	typedef void (*PassExecuteCallback)(void*);

	// Description of sync required for dependent RenderPass
	struct ExitSync
	{
		SyncType Type;
		UA64* NextPassFence;
	};

	// Data needed for executing RenderPass
	struct PassData
	{
		// Optional data used by RenderPass
		void* OptData = nullptr;
	};

	// Descriptor containing params for executing RenderPass
	struct PassDesc
	{
		// Sync required before execution of RenderPass
		SyncType EnterSync = SyncType::None;
		UA64 EnterFence1 = 0;
		UA64 EnterFence2 = 0;
		PassExecuteCallback Execute = nullptr;
		PassData* Data = nullptr;
		// Syncs required for dependent RenderPasses
		SyncType AllExitSyncs = SyncType::None;
		U8 DependentsCount = 0;
		ExitSync* ExitSyncs = nullptr;
	};
}