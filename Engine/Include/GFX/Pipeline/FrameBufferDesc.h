#pragma once
#include "GFX/Resource/State.h"
#include "GFX/QueueType.h"
#include "FrameResourceDesc.h"

namespace ZE::GFX::Pipeline
{
	// FrameBuffer creation data
	struct FrameBufferDesc
	{
		std::vector<std::string> ResourceNames;
		std::vector<FrameResourceDesc> ResourceInfo;
		std::vector<std::map<U64, Resource::State>> ResourceLifetimes;
		U64 TransitionLevelsCount;

		void Init(U64 resourceCount, const char* backbufferName, U32 backbufferWidth, U32 backbufferHeight);
		void AddResource(std::string&& name, FrameResourceDesc&& info);
	};
}