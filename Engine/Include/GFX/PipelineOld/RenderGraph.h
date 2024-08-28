#pragma once
#include "GFX/Graphics.h"
//#include "RendererBuildData.h"
//#include "RenderNode.h"
#include <bitset>
#include <thread>

namespace ZE::GFX::Pipeline
{
	// Type trait for checking whether given render graph is using backbuffer as an SRV
	template<typename T> struct IsBackbufferSRVInRenderGraph { static constexpr bool VALUE = false; };

	// Building and managing structure of render passes
	class RenderGraph
	{
		U64 levelCount = 0;
#if !_ZE_RENDER_GRAPH_SINGLE_THREAD
		U64 workersCount = 0;
		Ptr<std::pair<std::thread, ChainPool<CommandList>>> workerThreads;
#endif

	protected:

		U32 dynamicDataSize;
		//RendererExecuteData execData;

	public:
		RenderGraph(void* renderer, void* settingsData, void* dynamicData, U32 dynamicDataSize) noexcept;
		ZE_CLASS_DELETE(RenderGraph);

		void Execute(Graphics& gfx);
		void Free(Device& dev) noexcept;
	};
}