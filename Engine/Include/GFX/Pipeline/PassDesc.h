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

	// Descriptor containing params for executing RenderPass
	struct PassDesc
	{
		// Sync required before execution of RenderPass
		SyncType EnterSync = SyncType::None;
		UA64 EnterFence1 = 0;
		UA64 EnterFence2 = 0;
		// Pre-execute operations independent of render pass, ex. state transitions and sync points
		void (*Enter)() = nullptr;
		// RenderPass dependent execution
		PassExecuteCallback Execute = nullptr;
		// Optional data used by RenderPass
		void* Data = nullptr;
		// Post-execute operations independent of render pass, ex. state transitions
		void (*Exit)() = nullptr;
		// Syncs required for dependent RenderPasses
		SyncType AllExitSyncs = SyncType::None;
		U8 DependentsCount = 0;
		ExitSync* ExitSyncs = nullptr;
	};
}