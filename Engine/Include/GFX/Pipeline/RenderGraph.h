#pragma once
#include "RenderNode.h"
#include <bitset>

namespace ZE::GFX::Pipeline
{
	class RenderGraph final
	{
		U64 levelCount = 0;
		std::pair<PassDesc*, U64>* passes = nullptr;

		static void SortNodes(U64 currentNode, U64& orderedCount, const std::vector<std::vector<U64>>& graphList,
			std::vector<U64>& nodes, std::vector<std::bitset<2>>& visited);
		static void CullIndirectDependecies(U64 currentNode, U64 checkNode, U64 minDepLevel, std::vector<std::vector<U64>>& syncList,
			const std::vector<std::vector<U64>>& depList, const std::vector<U64>& dependencyLevels) noexcept;

	public:
		void Finalize(const std::vector<RenderNode>& nodes);

	public:
		RenderGraph();
		RenderGraph(RenderGraph&&) = delete;
		RenderGraph(const RenderGraph&) = delete;
		RenderGraph& operator=(RenderGraph&&) = delete;
		RenderGraph& operator=(const RenderGraph&) = delete;
		~RenderGraph();

		void Execute(Device& dev);
	};
}