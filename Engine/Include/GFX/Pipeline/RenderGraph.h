#pragma once
#include "GFX/Pipeline/FrameBuffer.h"
#include "RenderNode.h"
#include <bitset>

namespace ZE::GFX::Pipeline
{
	// Building and managing structure of render passes
	class RenderGraph
	{
		U64 levelCount = 0;
		std::pair<PassDesc*, U64>* passes = nullptr;

		static void SortNodes(U64 currentNode, U64& orderedCount, const std::vector<std::vector<U64>>& graphList,
			std::vector<U64>& nodes, std::vector<std::bitset<2>>& visited);
		static void CullIndirectDependecies(U64 currentNode, U64 checkNode, U64 minDepLevel, std::vector<std::vector<U64>>& syncList,
			const std::vector<std::vector<U64>>& depList, const std::vector<U64>& dependencyLevels) noexcept;

	protected:
		static constexpr const char* BACKBUFFER_NAME = "$backbuffer";

		FrameBuffer frameBuffer;

		void Finalize(Device& dev, SwapChain& swapChain, std::vector<RenderNode>& nodes, FrameBufferDesc& frameBufferDesc, bool minimizeDistances);

	public:
		RenderGraph() = default;
		RenderGraph(RenderGraph&&) = delete;
		RenderGraph(const RenderGraph&) = delete;
		RenderGraph& operator=(RenderGraph&&) = delete;
		RenderGraph& operator=(const RenderGraph&) = delete;
		virtual ~RenderGraph();

		void Execute(Device& dev);
	};
}