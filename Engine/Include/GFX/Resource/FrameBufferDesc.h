#pragma once
#include "PixelFormat.h"
#include "State.h"
#include "GFX/QueueType.h"

namespace ZE::GFX::Resource
{
	// Description of single resource in FrameBuffer
	struct FrameResourceDesc
	{
		U32 Width;
		U32 Height;
		U16 ArraySize;
		std::vector<PixelFormat> Formats;
	};

	struct FrameResourceLifetime
	{
		U64 StartDepLevel;
		std::map<U64, std::vector<std::pair<State, QueueType>>> States;
	};

	struct FrameBufferDesc
	{
		std::vector<std::string> ResourceNames;
		std::vector<FrameResourceDesc> ResourceInfo;
		std::vector<FrameResourceLifetime> ResourceLifetimes;

		void Reserve(U64 count) noexcept;
		void AddResource(std::string&& name, FrameResourceDesc&& info, U64 starLevel = -1);
	};
}