#pragma once
#include "RenderNode.h"
#include "GFX/Resource/FrameBuffer.h"
#include <bitset>

namespace ZE::GFX::Pipeline
{
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

		void Finalize(std::vector<RenderNode>& nodes, Resource::FrameBufferDesc& frameBufferDesc);

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