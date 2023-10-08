#pragma once
#include "GFX/Graphics.h"
#include "RendererBuildData.h"
#include "RenderNode.h"
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
		Ptr<std::pair<Ptr<PassDesc>, U64>> passes;
		Ptr<Ptr<PassCleanCallback>> passesCleaners;

		static void BeforeSync(Device& dev, const PassSyncDesc& syncInfo);
		static void AfterSync(Device& dev, PassSyncDesc& syncInfo);
		static void SortNodes(U64 currentNode, U64& orderedCount, const std::vector<std::vector<U64>>& graphList,
			std::vector<U64>& nodes, std::vector<std::bitset<2>>& visited);
		static void CullIndirectDependecies(U64 currentNode, U64 checkNode, U64 minDepLevel, std::vector<std::vector<U64>>& syncList,
			const std::vector<std::vector<U64>>& depList, const std::vector<U64>& dependencyLevels) noexcept;
		static void AssignState(Resource::State& presentState, Resource::State& currentState, RID rid, U64 depLevel);

		void ExecuteThread(Device& dev, CommandList& cl, PassDesc& pass);
		void ExecuteThreadSync(Device& dev, CommandList& cl, PassDesc& pass);

	protected:
		static constexpr U64 BACKBUFFER_RID = 0;

		U32 dynamicDataSize;
		RendererExecuteData execData;

		void Finalize(Device& dev, CommandList& mainList, std::vector<RenderNode>& nodes,
			FrameBufferDesc& frameBufferDesc, RendererBuildData& buildData, bool minimizeDistances);

	public:
		RenderGraph(void* renderer, void* settingsData, void* dynamicData, U32 dynamicDataSize) noexcept;
		ZE_CLASS_DELETE(RenderGraph);
		virtual ~RenderGraph() { ZE_ASSERT(passes == nullptr, "Render graph not freed before deletion!"); }

		constexpr Data::AssetsStreamer& GetAssetsStreamer() noexcept { return execData.Assets; }

		void Execute(Graphics& gfx);
		void Free(Device& dev) noexcept;
	};
}