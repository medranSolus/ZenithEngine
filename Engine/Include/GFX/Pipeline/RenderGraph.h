#pragma once
#include "GFX/Graphics.h"
#include "RendererBuildData.h"
#include "RenderNode.h"
#include <bitset>
#include <thread>

namespace ZE::GFX::Pipeline
{
	// Building and managing structure of render passes
	class RenderGraph
	{
		U64 levelCount = 0;
		U64 workersCount = 0;
		Ptr<std::pair<std::thread, CommandList>> workerThreads;
		Ptr<std::pair<Ptr<PassDesc>, U64>> passes;
		Ptr<PassDescStatic> staticPasses;
		Ptr<Ptr<PassCleanCallback>> passesCleaners;
		Ptr<Resource::PipelineStateGfx> sharedStates;

		static void BeforeSync(Device& dev, const PassSyncDesc& syncInfo);
		static void AfterSync(Device& dev, PassSyncDesc& syncInfo);
		static void SortNodes(U64 currentNode, U64& orderedCount, const std::vector<std::vector<U64>>& graphList,
			std::vector<U64>& nodes, std::vector<std::bitset<2>>& visited);
		static void CullIndirectDependecies(U64 currentNode, U64 checkNode, U64 minDepLevel, std::vector<std::vector<U64>>& syncList,
			const std::vector<std::vector<U64>>& depList, const std::vector<U64>& dependencyLevels) noexcept;

		void ExecuteThread(Device& dev, CommandList& cl, PassDesc& pass);

	protected:
		static constexpr U64 BACKBUFFER_RID = 0;

		FrameBuffer frameBuffer;
		Binding::Library bindings;

		void Finalize(Device& dev, CommandList& mainList, std::vector<RenderNode>& nodes,
			FrameBufferDesc& frameBufferDesc, RendererBuildData& buildData, bool minimizeDistances);

	public:
		RenderGraph() = default;
		ZE_CLASS_DELETE(RenderGraph);
		virtual ~RenderGraph();

		void Execute(Graphics& gfx);
	};
}