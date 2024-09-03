#pragma once
#include "GFX/Graphics.h"
#include "BuildResult.h"
#include "RenderGraphDesc.h"

namespace ZE::GFX::Pipeline
{
	// Additional flags to control final construction of render graph
	typedef U8 GraphFinalizeFlags;
	// Additional flags for controlling behavior during construction of render graph
	enum class GraphFinalizeFlag : GraphFinalizeFlags
	{
		// When initializing resources put their barriers on start of the frame or any preceeding execution group (default)
		InitializeResourcesFrameStart = 0x01,
		// When initializing resources put their barriers just before their first usage
		InitializeResourcesBeforePass = 0x02,
		// When initializing resources put their barriers where most of the barriers are currently performed
		InitializeResourcesWhereMostBarriers = 0x04,
		// By default initialization barriers are BarrierType::Immediate in chosen place
		// but this option performs split barrier between start of the execution group and first resource usage (if possible)
		InitializeResourcesSplitBarrier = 0x08,
		// Don't perform split barriers for transitions, use identified BarrierType::SplitBegin location
		NoSplitBarriersUseBegin = 0x10,
		// Don't perform split barriers for transitions, use identified BarrierType::SplitEnd location
		NoSplitBarriersUseEnd = 0x20,
		// In case of split barriers performed between 2 execution groups on same queue prefer using second group to place barriers
		CrossExecGroupSplitBarriersUseEndGroup = 0x40,
		// Don't transition backbuffer to the present state at the end of the graph execution
		NoPresentBarrier = 0x80,
	};
	ZE_ENUM_OPERATORS(GraphFinalizeFlag, GraphFinalizeFlags);

	// Builder class that should hold and manage intermediate form of render graph description with most of things precomputed
	// but allowing for updating whole graph in case of hard update. With soft updates it should allow for redirection of resources
	// and give sufficient info about it's transitions.
	//
	// Maybe should merge together with RenderGraph instead? Depends what relations between resources of both classes will be
	class RenderGraphBuilder final
	{
		struct GraphConnection
		{
			U32 NodeIndex;
			bool Required;
		};
		struct NodeDependency
		{
			std::vector<GraphConnection> PreceedingNodes;
		};
		struct NodeGroup
		{
			std::vector<NodeDependency> NodesDependecies;
		};
		struct PresenceInfo
		{
			bool Present = false;
			bool ProducerChecked = false;
			bool ActiveInputProducerPresent = false;
			bool ConsumerChecked = false;
			bool ActiveOutputProducerPresent = false;
			U32 NodeGroupIndex = 0;
		};
		struct ComputedNode
		{
			bool Present = false;
			U32 NodeGroupIndex = 0;
			std::vector<std::string> InputResources;
			std::vector<std::string> OutputResources;
		};

		FrameBufferFlags resourceOptions;
		Data::Library<std::string, FrameResourceDesc> resources;
		// Passes grouped by graph connector name
		std::vector<std::vector<RenderNode>> passDescs;
		// Render graph created via reversed adjacency list
		std::vector<NodeGroup> renderGraphDepList;
		// Topological order of render graph nodes
		std::vector<U32> topoplogyOrder;

		// Render graph created via adjacency list
		std::vector<ComputedNode> computedGraph;
		// Longest paths for each node
		std::vector<U32> dependencyLevels;
		// Final list of resources used in graph
		std::vector<std::string_view> computedResources;
		bool asyncComputeEnabled = false;
		U32 dependencyLevelCount = 0;

		bool IsGraphComputed() const noexcept { return computedGraph.size() && dependencyLevels.size() && computedResources.size() && dependencyLevelCount; }

		bool CheckNodeProducerPresence(U32 node, std::vector<PresenceInfo>& nodesPresence) const noexcept;
		bool CheckNodeConsumerPresence(U32 node, std::vector<PresenceInfo>& nodesPresence, const std::vector<std::vector<U32>>& graphList) const noexcept;
		bool SortNodesTopologyOrder(U32 currentNode, std::vector<std::bitset<2>>& visited) noexcept;
		BuildResult LoadGraphDesc(const RenderGraphDesc& desc) noexcept;
		BuildResult LoadResourcesDesc(const RenderGraphDesc& desc) noexcept;

		// Order: input, inner, output (without already present resources from inputs)
		std::unique_ptr<RID[]> GetNodeResources(U32 node) const noexcept;
		FrameBufferDesc GetFrameBufferLayout() const noexcept;
		void GroupRenderPasses(Device& dev, Data::AssetsStreamer& assets, class RenderGraph& graph, const RenderGraphDesc& desc) const;
		void ComputeGroupSyncs(class RenderGraph& graph) const noexcept;
		BuildResult FillPassBarriers(class RenderGraph& graph, GraphFinalizeFlags flags) noexcept;

	public:
		RenderGraphBuilder() = default;
		ZE_CLASS_MOVE(RenderGraphBuilder);
		~RenderGraphBuilder() = default;

		BuildResult LoadConfig(const RenderGraphDesc& desc) noexcept;

		BuildResult ComputeGraph(bool minimizeDistances = false) noexcept;
		BuildResult FinalizeGraph(Graphics& gfx, Data::AssetsStreamer& assets, class RenderGraph& graph, const RenderGraphDesc& desc, GraphFinalizeFlags flags = 0);

		void ClearConfig() noexcept;
		void ClearComputedGraph() noexcept;
	};
}