#pragma once
#include "GFX/Resource/CBuffer.h"
#include "RendererBuildData.h"
#include "RenderNode.h"
#include <bitset>
#include <thread>

namespace ZE::GFX::Pipeline
{
	// Building and managing structure of render passes
	class RenderGraph
	{
		U16 renderLevelCount = 0;
#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
		U16 gfxWorkersCount = 0;
		U16 computeWorkersCount = 0;
		Ptr<std::pair<std::thread, ChainPool<CommandList>>> workerThreadsGfx;
		Ptr<std::pair<std::thread, ChainPool<CommandList>>> workerThreadsCompute;
#endif
		Ptr<RenderLevel> levels;
		Ptr<std::array<Ptr<Ptr<PassCleanCallback>>, 2>> passCleaners;

		static void SortNodes(U64 currentNode, U64& orderedCount, const std::vector<std::vector<U64>>& graphList,
			std::vector<U64>& nodes, std::vector<std::bitset<2>>& visited);
		static void CullIndirectDependecies(U64 currentNode, U64 checkNode, U64 minDepLevel, std::vector<std::vector<U64>>& syncList,
			const std::vector<std::vector<U64>>& depList, const std::vector<U64>& dependencyLevels) noexcept;
		static void AssignState(Resource::State& presentState, Resource::State& currentState, RID rid, U64 depLevel);

	protected:
		static constexpr U64 BACKBUFFER_RID = 0;

		U32 dynamicDataSize;
		RendererExecuteData execData;

		void Finalize(Graphics& gfx, std::vector<RenderNode>& nodes, FrameBufferDesc& frameBufferDesc,
			RendererBuildData& buildData, bool minimizeDistances);

	public:
		RenderGraph(void* renderer, void* settingsData, void* dynamicData, U32 dynamicDataSize) noexcept;
		ZE_CLASS_DELETE(RenderGraph);
		virtual ~RenderGraph();

		constexpr Data::Storage& GetRegistry() noexcept { return execData.Registry; }
		constexpr Data::Storage& GetResources() noexcept { return execData.Resources; }

		void Execute(Graphics& gfx);
	};
}