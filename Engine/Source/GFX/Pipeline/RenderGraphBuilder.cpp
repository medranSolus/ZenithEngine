#include "GFX/Pipeline/RenderGraphBuilder.h"
#include "GFX/Pipeline/RenderGraph.h"

// Helper macro to end loading config and return when condition is true
#define ZE_CHECK_FAILED_CONFIG_LOAD(condition, result, message) do { if (condition) { ZE_FAIL(message); ClearConfig(dev); return BuildResult::##result; } } while (false)
// Helper macro to end computing graph and return when condition is true
#define ZE_CHECK_FAILED_GRAPH_COMPUTE(condition, result, message) do { if (condition) { ZE_FAIL(message); ClearComputedGraph(dev); return BuildResult::##result; } } while (false)

namespace ZE::GFX::Pipeline
{
	bool RenderGraphBuilder::CheckNodeProducerPresence(U32 node, std::vector<PresenceInfo>& nodesPresence) const noexcept
	{
		auto& presence = nodesPresence.at(node);
		// Check if found any present producer node
		PassExecutionType passType = passDescs.at(node).at(presence.NodeGroupIndex).GetExecType();
		if (presence.Present && passType == PassExecutionType::Producer)
			return true;
		// Otherwise traverse graph in search for producer nodes checking everything along the way
		if (!presence.ProducerChecked)
		{
			if (passType != PassExecutionType::Producer)
			{
				bool requiredConnectionsFound = true;
				for (const auto& prevNode : renderGraphDepList.at(node).NodesDependecies.at(presence.NodeGroupIndex).PreceedingNodes)
				{
					// In case no producer found for given connection then check if it's optional one
					if (!CheckNodeProducerPresence(prevNode.NodeIndex, nodesPresence))
					{
						if (prevNode.Required)
						{
							requiredConnectionsFound = false;
							break;
						}
					}
				}
				presence.ActiveInputProducerPresent = requiredConnectionsFound;
			}
			presence.ProducerChecked = true;
		}
		return presence.ActiveInputProducerPresent;
	}

	bool RenderGraphBuilder::CheckNodeConsumerPresence(U32 node, std::vector<PresenceInfo>& nodesPresence, const std::vector<std::vector<U32>>& graphList) const noexcept
	{
		auto& presence = nodesPresence.at(node);
		// Check if found any present producer node
		if (passDescs.at(node).at(presence.NodeGroupIndex).GetExecType() == PassExecutionType::Producer)
			return presence.Present;
		// Otherwise traverse graph in search for first producer node checking everything along the way
		if (!presence.ConsumerChecked)
		{
			bool outputConsumed = false;
			for (U32 nextNode : graphList.at(node))
			{
				if (CheckNodeConsumerPresence(nextNode, nodesPresence, graphList))
				{
					outputConsumed = true;
					break;
				}
			}

			// Check if last node in graph writes to swapchain, then keep it
			if (!outputConsumed && graphList.at(node).size() == 0)
			{
				const auto& outputRes = passDescs.at(node).at(presence.NodeGroupIndex).GetOutputResources();
				outputConsumed = std::find_if(outputRes.begin(), outputRes.end(), [](const auto& res) { return res == BACKBUFFER_NAME; }) != outputRes.end();
			}
			presence.ActiveOutputProducerPresent = outputConsumed;
			presence.ConsumerChecked = true;
		}
		return presence.ActiveOutputProducerPresent;
	}

	bool RenderGraphBuilder::SortNodesTopologyOrder(U32 currentNode, std::vector<std::bitset<2>>& visited) noexcept
	{
		if (visited.at(currentNode)[0])
		{
			ZE_ASSERT(!visited.at(currentNode)[1], "Found circular dependency in node [" +
				passDescs.at(currentNode).front().GetGraphConnectorName() + "]! Possible multiple outputs to same buffer.");
			return visited.at(currentNode)[1];
		}

		// Reverse order of traversal and order in topological order in reverse too by merging all connections to the single
		// graph connector group (all nodes) and treat their inputs as inputs to whole group
		visited.at(currentNode)[0] = true; // Mark as visited
		visited.at(currentNode)[1] = true; // Mark as visited in current scope of searching
		for (const auto& nodeGroup : renderGraphDepList.at(currentNode).NodesDependecies)
		{
			for (const auto& prevNode : nodeGroup.PreceedingNodes)
				if (SortNodesTopologyOrder(prevNode.NodeIndex, visited))
					return true;
		}
		visited.at(currentNode)[1] = false;
		topoplogyOrder.emplace_back(currentNode);
		return false;
	}

	BuildResult RenderGraphBuilder::LoadGraphDesc(Device& dev, const RenderGraphDesc& desc) noexcept
	{
		ZE_PERF_GUARD("RenderGraphBuilder::LoadGraphDesc");

		ZE_CHECK_FAILED_CONFIG_LOAD(desc.RenderPasses.size() != Utils::SafeCast<U32>(desc.RenderPasses.size()), ErrorTooManyPasses,
			"Number of passes cannot exceed UINT32_MAX!");

		// Gather passes and group them by graph connector names
		{
#if _ZE_RENDERER_CREATION_VALIDATION
			ZE_PERF_GUARD("RenderGraphBuilder::LoadGraphDesc - gather passes, renderer validation");
#else
			ZE_PERF_GUARD("RenderGraphBuilder::LoadGraphDesc - gather passes");
#endif
			for (U32 i = 0; i < desc.RenderPasses.size(); ++i)
			{
				const RenderNode& node = desc.RenderPasses.at(i);

				ZE_CHECK_FAILED_CONFIG_LOAD(!node.GetDesc().Execute, ErrorPassExecutionCallbackNotProvided,
					"Execution callback missing in [" + node.GetFullName() + "]!");
				ZE_CHECK_FAILED_CONFIG_LOAD(node.GetDesc().InitData && !node.GetDesc().FreeInitData, ErrorPassFreeInitDataCallbackNotProvided,
					"FreeInitData callback missing in [" + node.GetFullName() + "]!");
				ZE_CHECK_FAILED_CONFIG_LOAD(node.GetDesc().InitData && !node.GetDesc().CopyInitData, ErrorPassCopyInitDataCallbackNotProvided,
					"CopyInitData callback missing in [" + node.GetFullName() + "]!");
				ZE_CHECK_FAILED_CONFIG_LOAD(node.GetDesc().InitData && !node.GetDesc().Init, ErrorPassInitCallbackNotProvided,
					"Init callback missing in [" + node.GetFullName() + "] while initialization data has been provided!");

				// Check if pass with same connector name is not already in the database
				bool newPass = true;
				for (U32 j = 0; j < passDescs.size(); ++j)
				{
					if (passDescs.at(j).front().GetGraphConnectorName() == node.GetGraphConnectorName())
					{
#if _ZE_RENDERER_CREATION_VALIDATION
						bool wrongOutputSet = false, notSorted = true;
						std::vector<std::string> nodeOutputs;
						for (const auto& setNode : passDescs.at(j))
						{
							if (setNode.GetOutputs().size() != node.GetOutputs().size())
							{
								wrongOutputSet = true;
								break;
							}
							// If sizes match then need to check if output sets are the same
							if (notSorted)
							{
								nodeOutputs = node.GetOutputs();
								std::sort(nodeOutputs.begin(), nodeOutputs.end());
								notSorted = false;
							}
							std::vector<std::string> currentOutputs = setNode.GetOutputs();
							std::sort(currentOutputs.begin(), currentOutputs.end());
							if (nodeOutputs != currentOutputs)
							{
								wrongOutputSet = true;
								break;
							}
						}
						ZE_CHECK_FAILED_CONFIG_LOAD(wrongOutputSet, ErrorPassWrongOutputSet,
							"Output resources of the [" + node.GetFullName() + "] doesn't match with rest of the passes with same graph connector name!");

						if (passDescs.at(j).size() > 0)
						{
							// Check for name correctness
							ZE_CHECK_FAILED_CONFIG_LOAD(node.GetPassName() == "", ErrorPassEmptyName,
								"Passes sharing same connector name of [" + node.GetGraphConnectorName() +
								"] are required to have it's own distinct name!");
							// Check for first pass since this name is required only when there are multiple passes in graph node
							if (passDescs.at(j).size() == 1)
							{
								ZE_CHECK_FAILED_CONFIG_LOAD(passDescs.at(j).front().GetPassName() == "", ErrorPassEmptyName,
									"Passes sharing same connector name of [" + passDescs.at(j).front().GetGraphConnectorName() +
									"] are required to have it's own disting name!");
							}
							// Check for name clashes with all passes
							for (const auto& sharedNode : passDescs.at(j))
							{
								ZE_CHECK_FAILED_CONFIG_LOAD(node.GetPassName() == sharedNode.GetPassName(), ErrorPassNameClash,
									"Pass name [" + node.GetFullName() + "] is not unique!");
							}
						}
						// Check if not all inputs are optional
						if (node.GetExecType() != PassExecutionType::Producer)
						{
							bool allInputsOptional = true;
							for (bool inputRequired : node.GetInputRequirements())
							{
								if (inputRequired)
								{
									allInputsOptional = false;
									break;
								}
							}
							ZE_CHECK_FAILED_CONFIG_LOAD(allInputsOptional, ErrorProcessorAllInputsOptional,
								"Only producer passes can have all optional input, pass [" + node.GetFullName() +
								"] need to have at least one required input resource!");
						}
#endif
						// Append new pass as node stays the same
						auto& newNode = passDescs.at(j).emplace_back(node);
						// If outputs to backbuffer then change to producer
						if (newNode.GetExecType() != PassExecutionType::Producer && std::find(newNode.GetOutputResources().begin(), newNode.GetOutputResources().end(), BACKBUFFER_NAME) != newNode.GetOutputResources().end())
							newNode.SetProducer();
						newPass = false;
						break;
					}
				}
				if (newPass)
					passDescs.emplace_back().emplace_back(node);
			}
		}

		// Init dependency info for all nodes in given group
		renderGraphDepList.resize(passDescs.size());
		for (U32 i = 0; i < passDescs.size(); ++i)
			renderGraphDepList.at(i).NodesDependecies.resize(passDescs.at(i).size());

		// Create graph via reversed adjacency list (list of node groups and for each node in a group
		// list of nodes from which traversal is possible as data flow)
		{
			ZE_PERF_GUARD("RenderGraphBuilder::LoadGraphDesc - graph creation");
			for (U32 i = 0; i < passDescs.size(); ++i)
			{
				const auto& graphNodeGroup = passDescs.at(i);
				for (U32 j = 0; j < graphNodeGroup.size(); ++j)
				{
					const auto& graphNode = graphNodeGroup.at(j);

					// Add connection between passes if manually scheduled
					if (graphNode.GetPreceedingPass() != "")
					{
						auto& preceedingNodes = renderGraphDepList.at(i).NodesDependecies.at(j).PreceedingNodes;
						for (U32 k = 0; k < passDescs.size(); ++k)
						{
							const auto& checkedGroup = passDescs.at(k);
							if (checkedGroup.front().GetGraphConnectorName() == graphNode.GetPreceedingPass())
							{
								auto connection = std::find_if(preceedingNodes.begin(), preceedingNodes.end(), [k](const GraphConnection& connection) { return connection.NodeIndex == k; });
								if (connection == preceedingNodes.end())
									preceedingNodes.emplace_back(k, true);
								else
									connection->Required = true;
								break;
							}
						}
					}

					// Connect based on which outputs are present in other nodes as inputs (directed graph)
					for (const auto& out : graphNode.GetOutputs())
					{
						for (U32 k = 0; k < passDescs.size(); ++k)
						{
							const auto& checkedGroup = passDescs.at(k);
							for (U32 l = 0; l < checkedGroup.size(); ++l)
							{
								const auto& checkedNode = checkedGroup.at(l);
								auto it = std::find(checkedNode.GetInputs().begin(), checkedNode.GetInputs().end(), out);
								if (it != checkedNode.GetInputs().end())
								{
									ZE_CHECK_FAILED_CONFIG_LOAD(i == k, ErrorOutputAsInputSamePass,
										"Output resource [" + out + "] specified as input " + (graphNodeGroup.size() == 1 ? " of the same pass!"
											: " of pass with the same graph connector name [" + graphNode.GetFullName() + "]!"));

									ResIndex inputIdx = Utils::SafeCast<ResIndex>(std::distance(checkedNode.GetInputs().begin(), it));
									auto& preceedingNodes = renderGraphDepList.at(k).NodesDependecies.at(l).PreceedingNodes;
									auto connection = std::find_if(preceedingNodes.begin(), preceedingNodes.end(), [i](const GraphConnection& connection) { return connection.NodeIndex == i; });

									// Specifying only graph group index as outputs in given group must be the same, only inputs to all nodes can vary
									if (connection == preceedingNodes.end())
										preceedingNodes.emplace_back(i, checkedNode.IsInputRequired(inputIdx));
									else
										connection->Required |= checkedNode.IsInputRequired(inputIdx);
								}
							}
						}
					}
				}
			}
		}

		// Sort nodes in topological order
		{
			ZE_PERF_GUARD("RenderGraphBuilder::LoadGraphDesc - topology sort");
			topoplogyOrder.reserve(passDescs.size());
			// Visited | on current stack of graph traversal
			std::vector<std::bitset<2>> visited(passDescs.size(), 0);
			for (U32 i = 0; i < passDescs.size(); ++i)
			{
				if (!visited.at(i)[0])
				{
					ZE_CHECK_FAILED_CONFIG_LOAD(SortNodesTopologyOrder(i, visited), ErrorPassCircularDependency,
						"Ill-formed render graph! Cycle found between nodes, aborting loading config.");
				}
			}
		}
		return BuildResult::Success;
	}

	BuildResult RenderGraphBuilder::LoadResourcesDesc(Device& dev, const RenderGraphDesc& desc) noexcept
	{
		ZE_PERF_GUARD("RenderGraphBuilder::LoadResourcesDesc");

		// Find list of resources that are in the config (they will form RIDs later on)
		std::vector<std::string_view> presentResources;
		for (const auto& res : desc.Resources)
		{
			ZE_CHECK_FAILED_CONFIG_LOAD((res.second.Flags & (FrameResourceFlag::ForceRTV | FrameResourceFlag::ForceUAV)) && (res.second.Flags & FrameResourceFlag::ForceDSV),
				ErrorIncorrectResourceUsage, "Cannot create depth stencil together with render target or unordered access view for same resource [" + res.first + "]!");

			ZE_CHECK_FAILED_CONFIG_LOAD((res.second.Flags & FrameResourceFlag::ForceRTV) && Utils::IsDepthStencilFormat(res.second.Format),
				ErrorIncorrectResourceFormat, "Cannot use depth stencil format with render target for resource [" + res.first + "]!");

			ZE_CHECK_FAILED_CONFIG_LOAD((res.second.Type != FrameResourceType::Texture2D) && (res.second.Flags & FrameResourceFlag::ForceDSV),
				ErrorWrongResourceConfiguration, "Cannot create non-2D texture as depth stencil in resource [" + res.first + "]!");

			ZE_CHECK_FAILED_CONFIG_LOAD((res.second.Flags & FrameResourceFlag::SimultaneousAccess) && (res.second.Flags & FrameResourceFlag::ForceDSV),
				ErrorWrongResourceConfiguration, "Simultaneous access cannot be used on depth stencil in resource [" + res.first + "]!");

			if (res.first == BACKBUFFER_NAME)
				presentResources.emplace_back(res.first);
			else
			{
				for (const auto& pass : desc.RenderPasses)
				{
					if (std::find(pass.GetOutputResources().begin(), pass.GetOutputResources().end(), res.first) != pass.GetOutputResources().end()
						|| std::find(pass.GetOutputReplacementResources().begin(), pass.GetOutputReplacementResources().end(), res.first) != pass.GetOutputReplacementResources().end())
					{
						presentResources.emplace_back(res.first);
						break;
					}
				}
			}
		}
		ZE_ASSERT_WARN(presentResources.size() == desc.Resources.size(), "Some frame resources are not accesed and will be removed from pipeline config!");

		// Add present resources along with inner buffers as possible super-set of future FrameBuffer resources
		for (const auto& resName : presentResources)
		{
			auto element = std::find_if(desc.Resources.begin(), desc.Resources.end(), [&resName](const std::pair<std::string, FrameResourceDesc>& x) { return x.first == resName; });
			ZE_ASSERT(element != desc.Resources.end(), "All resource names should be present at this point!");

			resources.Add(element->first, element->second);
		}
		for (const auto& pass : desc.RenderPasses)
		{
			for (ResIndex i = 0, size = Utils::SafeCast<ResIndex>(pass.GetInnerBuffers().size()); i < size; ++i)
				resources.Add(pass.GetInnerBufferName(i), pass.GetInnerBuffers().at(i));
		}
		// Sanity check if not exceeding max RID
		ZE_CHECK_FAILED_CONFIG_LOAD(resources.Size() >= INVALID_RID, ErrorTooManyResources,
			"Exceeded max number of resources that can be created for the scene!");

		resourceOptions = desc.ResourceOptions;
		return BuildResult::Success;
	}

	std::unique_ptr<RID[]> RenderGraphBuilder::GetNodeResources(U32 node) const noexcept
	{
		const auto& computed = computedGraph.at(node);
		std::vector<std::string_view> out;
		for (const auto& output : computed.OutputResources)
		{
			auto it = std::find(computed.InputResources.begin(), computed.InputResources.end(), output);
			if (it == computed.InputResources.end() || output == "")
				out.emplace_back(output);
		}

		const auto& renderNode = passDescs.at(node).at(computed.NodeGroupIndex);
		auto rids = std::make_unique<RID[]>(computed.InputResources.size() + renderNode.GetInnerBuffers().size() + out.size());
		RID i = 0;
		for (const auto& input : computed.InputResources)
		{
			RID rid = INVALID_RID;
			if (input != "")
			{
				auto it = std::find(computedResources.begin(), computedResources.end(), input);
				if (it != computedResources.end())
					rid = Utils::SafeCast<RID>(std::distance(computedResources.begin(), it));
				ZE_ASSERT(rid != INVALID_RID, "If input is not empty it must always be present after computing graph!");
			}
			rids[i++] = rid;
		}
		for (ResIndex inner = 0, size = Utils::SafeCast<ResIndex>(renderNode.GetInnerBuffers().size()); inner < size; ++inner)
		{
			RID rid = INVALID_RID;
			auto it = std::find(computedResources.begin(), computedResources.end(), renderNode.GetInnerBufferName(inner));
			if (it != computedResources.end())
				rid = Utils::SafeCast<RID>(std::distance(computedResources.begin(), it));
			ZE_ASSERT(rid != INVALID_RID, "Inner buffers must always be present after computing graph!");
			rids[i++] = rid;
		}
		for (const auto& output : out)
		{
			RID rid = INVALID_RID;
			if (output != "")
			{
				auto it = std::find(computedResources.begin(), computedResources.end(), output);
				if (it != computedResources.end())
					rid = Utils::SafeCast<RID>(std::distance(computedResources.begin(), it));
				ZE_ASSERT(rid != INVALID_RID, "If output is not empty it must always be present after computing graph!");
			}
			rids[i++] = rid;
		}
		return rids;
	}

	FrameBufferDesc RenderGraphBuilder::GetFrameBufferLayout(const RenderGraph& graph) const noexcept
	{
		ZE_PERF_GUARD("RenderGraphBuilder::GetFrameBufferLayout");

		FrameBufferDesc desc = {};
		desc.Flags = resourceOptions;
		desc.PassLevelCount = dependencyLevelCount;

		// Begin | End level
		std::unordered_map<std::string_view, std::pair<U32, U32>> resourceLookup;
		resourceLookup.reserve(computedResources.size());
		desc.Resources.reserve(computedResources.size() + graph.ffxInternalBuffers.Size());
		for (const auto& resName : computedResources)
		{
			const auto& res = resources.Get(std::string(resName));
			desc.Resources.emplace_back(res);
			resourceLookup.emplace(resName, std::make_pair<U32, U32>(UINT32_MAX, 0U));
			if (res.Flags & FrameResourceFlag::Temporal)
				resourceLookup.at(resName) = { 0U, dependencyLevelCount };
		}

		// Compute resource lifetimes based on dependency levels
		for (U32 i = 0; i < computedGraph.size(); ++i)
		{
			const auto& computed = computedGraph.at(i);
			if (computed.Present)
			{
				U32 depStart = dependencyLevels.at(i);
				U32 depEnd = depStart + 1;

				for (const auto& input : computed.InputResources)
				{
					if (input != "")
					{
						auto& lifetime = resourceLookup.at(input);
						if (lifetime.first > depStart)
							lifetime.first = depStart;
						if (lifetime.second < depEnd)
							lifetime.second = depEnd;
					}
				}
				for (const auto& output : computed.OutputResources)
				{
					if (output != "")
					{
						auto& lifetime = resourceLookup.at(output);
						if (lifetime.first > depStart)
							lifetime.first = depStart;
						if (lifetime.second < depEnd)
							lifetime.second = depEnd;
					}
				}

				// Check temporary inner resources
				const auto& node = passDescs.at(i).at(computed.NodeGroupIndex);
				for (ResIndex j = 0, size = Utils::SafeCast<ResIndex>(node.GetInnerBuffers().size()); j < size; ++j)
				{
					auto& lifetime = resourceLookup.at(node.GetInnerBufferName(j));
					if (lifetime.first > depStart)
						lifetime.first = depStart;
					if (lifetime.second < depEnd)
						lifetime.second = depEnd;
				}
			}
		}

		// Copy lifetimes to final structure
		desc.ResourceLifetimes.clear();
		desc.ResourceLifetimes.reserve(desc.Resources.size());
		for (const auto& resName : computedResources)
			desc.ResourceLifetimes.emplace_back(resourceLookup.at(resName));

		// Get info from FFX buffers
		graph.ffxInternalBuffers.Iter([&](auto& ffxRes)
			{
				desc.Resources.emplace_back(ffxRes.Desc).Flags |= FrameResourceFlag::InternalResourceActive;
				if (ffxRes.PassID != UINT32_MAX && !(ffxRes.Desc.Flags & FrameResourceFlag::Temporal))
				{
					U32 depStart = dependencyLevels.at(ffxRes.PassID);
					desc.ResourceLifetimes.emplace_back(depStart, depStart + 1);
				}
				else
					desc.ResourceLifetimes.emplace_back(0U, dependencyLevelCount);
			});
		return desc;
	}

	void RenderGraphBuilder::GroupRenderPasses(Device& dev, RenderGraph& graph) const
	{
		ZE_PERF_GUARD("RenderGraphBuilder::GroupRenderPasses");

		auto fillInPassData = [&](U32 node, RenderGraph::ParallelPassGroup::PassInfo& passInfo)
			{
				const PassDesc& nodeDesc = passDescs.at(node).at(computedGraph.at(node).NodeGroupIndex).GetDesc();

				passInfo.PassID = node;
				passInfo.Exec = nodeDesc.Execute;
				passInfo.Resources = GetNodeResources(node);
				passInfo.Data.Resources = passInfo.Resources.get();
			};

		if (asyncComputeEnabled)
		{
			ZE_PERF_GUARD("RenderGraphBuilder::GroupRenderPasses - async compute enabled");
			graph.asyncListChain.Exec([&dev](CommandList& cmd) { cmd.Init(dev, QueueType::Compute); });

			// Create exec groups for every dependency level
			std::vector<std::vector<std::vector<U32>>> execGroups(dependencyLevelCount);
			std::vector<std::vector<std::vector<U32>>> execGroupsAsync(dependencyLevelCount);
			// 1 - GFX, 2 - Async, 3 - both
			std::vector<U8> execGroupWorkType(dependencyLevelCount);
			for (U32 i = 0, size = Utils::SafeCast<U32>(dependencyLevels.size()); i < size; ++i)
			{
				if (computedGraph.at(i).Present)
				{
					U32 depLevel = dependencyLevels.at(i);
					bool isAsync = passDescs.at(i).at(computedGraph.at(i).NodeGroupIndex).IsAsync();
					execGroupWorkType.at(depLevel) |= (U32)!isAsync | (((U32)isAsync) << 1);
					if (isAsync)
					{
						if (execGroupsAsync.at(depLevel).size() == 0)
							execGroupsAsync.at(depLevel).emplace_back();
						execGroupsAsync.at(depLevel).back().emplace_back(i);
					}
					else
					{
						if (execGroups.at(depLevel).size() == 0)
							execGroups.at(depLevel).emplace_back();
						execGroups.at(depLevel).back().emplace_back(i);
					}
				}
			}

			// Merge if exec groups have same work characteristics
			auto mergeGroup = [](U32 i, std::vector<std::vector<std::vector<U32>>>& execGroups)
				{
					auto& currentGroup = execGroups.at(i);
					auto& nextGroup = execGroups.at(i + 1);
					currentGroup.reserve(currentGroup.size() + nextGroup.size());
					currentGroup.insert(currentGroup.end(), nextGroup.begin(), nextGroup.end());
					execGroups.erase(execGroups.begin() + i + 1);
				};
			for (U32 i = dependencyLevelCount - 1; i > 0;)
			{
				--i;
				if (execGroupWorkType.at(i) == execGroupWorkType.at(i + 1))
				{
					mergeGroup(i, execGroups);
					mergeGroup(i, execGroupsAsync);
					execGroupWorkType.erase(execGroupWorkType.begin() + i + 1);
				}
			}
			execGroupWorkType.clear();
			graph.execGroupCount = Utils::SafeCast<U32>(execGroups.size());
			graph.passExecGroups = std::make_unique<std::array<RenderGraph::ExecutionGroup, 2>[]>(graph.execGroupCount);

			// Move passes to correct execution groups for both queues
			auto fillInExecGroup = [&](U32 i, RenderGraph::ExecutionGroup& execGroup, std::vector<std::vector<std::vector<U32>>>& execGroups)
				{
					execGroup.PassGroupCount = Utils::SafeCast<U32>(execGroups.at(i).size());

					if (execGroup.PassGroupCount)
					{
						execGroup.PassGroups = std::make_unique<RenderGraph::ParallelPassGroup[]>(execGroup.PassGroupCount);
						for (U32 j = 0; j < execGroup.PassGroupCount; ++j)
						{
							auto& passGroup = execGroup.PassGroups[j];
							passGroup.PassCount = Utils::SafeCast<U32>(execGroups.at(i).at(j).size());

							if (passGroup.PassCount)
							{
								passGroup.Passes = std::make_unique<RenderGraph::ParallelPassGroup::PassInfo[]>(passGroup.PassCount);
								for (U32 k = 0; k < passGroup.PassCount; ++k)
								{
									fillInPassData(execGroups.at(i).at(j).at(k), passGroup.Passes[k]);
								}
							}
						}
					}
				};
			for (U32 i = 0; i < graph.execGroupCount; ++i)
			{
				fillInExecGroup(i, graph.passExecGroups[i].at(0), execGroups);
				fillInExecGroup(i, graph.passExecGroups[i].at(1), execGroupsAsync);
			}
		}
		else
		{
			ZE_PERF_GUARD("RenderGraphBuilder::GroupRenderPasses - no async compute");
			graph.execGroupCount = 1;
			graph.passExecGroups = std::make_unique<std::array<RenderGraph::ExecutionGroup, 2>[]>(1);

			auto& execGroup = graph.passExecGroups[0].at(0);
			execGroup.PassGroupCount = dependencyLevelCount;
			execGroup.PassGroups = std::make_unique<RenderGraph::ParallelPassGroup[]>(dependencyLevelCount);

			// Get number of passes in each group and assign passes to those groups
			for (U32 i = 0, size = Utils::SafeCast<U32>(dependencyLevels.size()); i < size; ++i)
			{
				if (computedGraph.at(i).Present)
					++execGroup.PassGroups[dependencyLevels.at(i)].PassCount;
			}
			auto passesIndices = std::make_unique<U32[]>(dependencyLevelCount);
			for (U32 level = 0; level < dependencyLevelCount; ++level)
			{
				passesIndices[level] = execGroup.PassGroups[level].PassCount;
				execGroup.PassGroups[level].Passes = std::make_unique<RenderGraph::ParallelPassGroup::PassInfo[]>(execGroup.PassGroups[level].PassCount);
			}

			// Fill in description of single pass in group
			for (U32 i = 0, size = Utils::SafeCast<U32>(dependencyLevels.size()); i < size; ++i)
			{
				if (computedGraph.at(i).Present)
				{
					U32 depLevel = dependencyLevels.at(i);
					auto& passGroup = execGroup.PassGroups[depLevel];
					U32 passIndex = passGroup.PassCount - passesIndices[depLevel]--;

					fillInPassData(i, passGroup.Passes[passIndex]);
				}
			}
		}
	}

	void RenderGraphBuilder::InitializeRenderPasses(Device& dev, Data::AssetsStreamer& assets, RenderGraph& graph, const RenderGraphDesc& desc)
	{
		ZE_PERF_GUARD("RenderGraphBuilder::InitializeRenderPasses");

		bool cascadeUpdate = false;
		RendererPassBuildData buildData = { graph.execData.Bindings, assets, graph.ffxInterface, desc.SettingsRange, desc.DynamicDataRange, desc.Samplers };
		auto fillInitData = [&](RenderGraph::ExecutionGroup& execGroup)
			{
				for (U32 i = 0; i < execGroup.PassGroupCount; ++i)
				{
					for (U32 j = 0; j < execGroup.PassGroups[i].PassCount; ++j)
					{
						auto& pass = execGroup.PassGroups[i].Passes[j];
						auto& computed = computedGraph.at(pass.PassID);
						auto& node = passDescs.at(pass.PassID).at(computed.NodeGroupIndex);

						FFX::PassInfo ffxPassInfo = {};
						ffxPassInfo.PassID = pass.PassID;
						FFX::SetCurrentPass(graph.ffxInterface, &ffxPassInfo);

						// Always compute exec data for invalid passes
						if (node.GetDesc().Type == CorePassType::Invalid || node.IsExecDataCachingDisabled())
						{
							pass.Data.ExecData = node.GetDesc().Init(dev, buildData, node.GetDesc().InitializeFormats, node.GetDesc().InitData);
							graph.passExecData.emplace_back(pass.Data.ExecData, node.GetDesc().Clean);
						}
						else
						{
							// If pass has been created before then only perform update, otherwise create from start
							std::string fullname = node.GetFullName();
							if (!execDataCache.Contains(fullname))
								execDataCache.Add(fullname, nullptr, node.GetDesc().Clean);

							auto& execData = execDataCache.Get(fullname);
							if (execData.first == nullptr)
							{
								if (node.GetDesc().Init)
									execData.first = node.GetDesc().Init(dev, buildData, node.GetDesc().InitializeFormats, node.GetDesc().InitData);
							}
							else if (node.GetDesc().Update)
								cascadeUpdate |= node.GetDesc().Update(dev, buildData, execData.first, node.GetDesc().InitializeFormats);
							pass.Data.ExecData = execData.first;
						}
						FFX::SetCurrentPass(graph.ffxInterface, nullptr);
					}
				}
			};
		for (U32 group = 0; group < graph.execGroupCount; ++group)
		{
			fillInitData(graph.passExecGroups[group].at(0));
			fillInitData(graph.passExecGroups[group].at(1));
		}

		// Remove any exec data from passes that are not present
		auto removeNodeData = [&](RenderNode& node)
			{
				std::string fullname = node.GetFullName();
				if (execDataCache.Contains(fullname))
				{
					auto& execData = execDataCache.Get(fullname);
					if (execData.first)
						execData.second(dev, execData.first);
					execDataCache.Remove(fullname);
				}
			};
		for (U32 passId = 0; passId < passDescs.size(); ++passId)
		{
			auto& computed = computedGraph.at(passId);
			if (!computed.Present)
			{
				for (auto& node : passDescs.at(passId))
					removeNodeData(node);
			}
			else if (passDescs.at(passId).size() > 1)
			{
				for (U32 index = 0; index < computed.NodeGroupIndex; ++index)
					removeNodeData(passDescs.at(passId).at(index));
				for (U32 index = computed.NodeGroupIndex + 1; index < passDescs.at(passId).size(); ++index)
					removeNodeData(passDescs.at(passId).at(index));
			}
		}

		// Perform updates as long as there is required to reapply any changes that might need rebuilding render grap
		while (cascadeUpdate)
		{
			cascadeUpdate = false;
			for (U32 passId = 0; passId < passDescs.size(); ++passId)
			{
				auto& computed = computedGraph.at(passId);
				if (computed.Present)
				{
					auto& node = passDescs.at(passId).at(computed.NodeGroupIndex);

					if (node.GetDesc().Update)
					{
						std::string fullname = node.GetFullName();
						if (execDataCache.Contains(fullname))
							cascadeUpdate |= node.GetDesc().Update(dev, buildData, execDataCache.Get(fullname).first, node.GetDesc().InitializeFormats);
					}
				}
			}
		}

		RID ffxBuffersOffset = Utils::SafeCast<RID>(computedResources.size());
		graph.ffxInternalBuffers.Transform([&ffxBuffersOffset](FFX::InternalResourceDescription& desc) { desc.ResID = ffxBuffersOffset++; });

		// After pass data have been scheduled to upload we can start actual GPU upload request
		assets.GetDisk().StartUploadGPU(true);
		// Clear up loaded shaders
		buildData.FreeShaderCache(dev);
		graph.ffxBuffersChanged = false;
	}

	void RenderGraphBuilder::ComputeGroupSyncs(class RenderGraph& graph) const noexcept
	{
		if (asyncComputeEnabled && graph.execGroupCount > 1)
		{
			ZE_PERF_GUARD("RenderGraphBuilder::ComputeGroupSyncs");
			auto checkGroupSync = [&](RenderGraph::ExecutionGroup& group, RenderGraph::ExecutionGroup& prevGroup)
				{
					// Skip if no need to sync with anything
					if (prevGroup.PassGroupCount)
					{
						bool activeDependency = false;
						for (U32 passGroup = 0; passGroup < group.PassGroupCount && !activeDependency; ++passGroup)
						{
							for (U32 pass = 0; pass < group.PassGroups[passGroup].PassCount && !activeDependency; ++pass)
							{
								U32 passId = group.PassGroups[passGroup].Passes[pass].PassID;
								auto& dependecies = renderGraphDepList.at(passId).NodesDependecies.at(computedGraph.at(passId).NodeGroupIndex);

								// Check pass groups from the end since it's higher chance to find syncing node
								for (U32 prevPassGroup = prevGroup.PassGroupCount; prevPassGroup > 0 && !activeDependency;)
								{
									--prevPassGroup;
									for (U32 prevPass = prevGroup.PassGroups[prevPassGroup].PassCount; prevPass > 0;)
									{
										--prevPass;
										U32 prevPassId = prevGroup.PassGroups[prevPassGroup].Passes[prevPass].PassID;
										for (const auto& dep : dependecies.PreceedingNodes)
										{
											if (prevPassId == dep.NodeIndex)
											{
												activeDependency = true;
												break;
											}
										}
									}
								}
							}
						}
						if (activeDependency)
						{
							group.QueueWait = true;
							prevGroup.SignalFence = &group.WaitFence;
						}
					}
				};
			for (U32 i = 1; i < graph.execGroupCount; ++i)
			{
				checkGroupSync(graph.passExecGroups[i].at(0), graph.passExecGroups[i - 1].at(1));
				checkGroupSync(graph.passExecGroups[i].at(1), graph.passExecGroups[i - 1].at(0));
			}
		}
	}

	BuildResult RenderGraphBuilder::FillPassBarriers(Device& dev, RenderGraph& graph, GraphFinalizeFlags flags) noexcept
	{
		ZE_PERF_GUARD("RenderGraphBuilder::FillPassBarriers");

		struct ResourceState
		{
			TextureLayout InputLayout;
			TextureLayout OutputLayout;
			ResourceAccesses PossibleAccess;
			bool WorkGfx;
			bool WorkCompute;
			bool WorkRayTracing;
			bool AsyncQueue;
			U32 ExecGroupIndex;
			U32 PassGroupIndex;

			constexpr RenderGraph::ExecutionGroup& GetExecGroup(RenderGraph& graph) const noexcept { ZE_ASSERT(ExecGroupIndex < graph.execGroupCount, "Incorrect exec group index!"); return graph.passExecGroups[ExecGroupIndex].at(static_cast<U64>(AsyncQueue)); }
			constexpr RenderGraph::ParallelPassGroup& GetPassGroup(RenderGraph& graph, RenderGraph::ExecutionGroup& execGroup) const noexcept { ZE_ASSERT(PassGroupIndex < execGroup.PassGroupCount, "Incorrect pass group index!"); return execGroup.PassGroups[PassGroupIndex]; }
			constexpr RenderGraph::ParallelPassGroup& GetPassGroup(RenderGraph& graph) const noexcept { return GetPassGroup(graph, GetExecGroup(graph)); }
		};

		// Map for quick resolution of RIDs
		std::unordered_map<std::string_view, RID> resourceLookup;
		resourceLookup.reserve(computedResources.size());
		for (RID i = 0; i < computedResources.size(); ++i)
			resourceLookup.emplace(computedResources.at(i), i);

		// Group all resources into lifetimes based on their usage
		std::vector<std::map<U32, ResourceState>> resourceLifetimes;
		{
			ZE_PERF_GUARD("RenderGraphBuilder::FillPassBarriers - group lifetimes");

			resourceLifetimes.resize(computedResources.size());
			auto gatherResourceUsage = [&](RenderGraph::ExecutionGroup& execGroup, U32 execGroupIndex, bool async) -> BuildResult
				{
					auto mergeReadOnlyLayout = [](TextureLayout& current, TextureLayout incoming) -> bool
						{
							// If possible then convert common layouts to something more
							// generic in case of read only texture layouts
							bool layoutMismatch = true;
							switch (incoming)
							{
							case TextureLayout::Common:
							{
								switch (current)
								{
								case TextureLayout::GenericRead:
								case TextureLayout::ShaderResource:
								case TextureLayout::CopySource:
								{
									layoutMismatch = false;
									current = TextureLayout::Common;
									break;
								}
								default:
									break;
								}
								break;
							}
							case TextureLayout::GenericRead:
							case TextureLayout::ShaderResource:
							case TextureLayout::CopySource:
							{
								switch (current)
								{
								case TextureLayout::Common:
								{
									layoutMismatch = false;
									current = TextureLayout::Common;
									break;
								}
								case TextureLayout::GenericRead:
								case TextureLayout::ShaderResource:
								case TextureLayout::CopySource:
								{
									layoutMismatch = false;
									current = TextureLayout::GenericRead;
									break;
								}
								default:
									break;
								}
								break;
							}
							default:
								break;
							}
							return layoutMismatch;
						};
					for (U32 j = 0; j < execGroup.PassGroupCount; ++j)
					{
						auto& passGroup = execGroup.PassGroups[j];

						for (U32 k = 0; k < passGroup.PassCount; ++k)
						{
							auto& pass = passGroup.Passes[k];
							auto& computed = computedGraph.at(pass.PassID);
							auto& renderNode = passDescs.at(pass.PassID).at(computed.NodeGroupIndex);
							U32 depLevel = dependencyLevels.at(pass.PassID);

							// Go over all input, output and internal resources to asign their layouts
							for (ResIndex input = 0; input < computed.InputResources.size(); ++input)
							{
								std::string_view name = computed.InputResources.at(input);

								if (name != "")
								{
									auto& lifetime = resourceLifetimes.at(resourceLookup.at(name));
									TextureLayout layout = renderNode.GetInputLayout(input);

									if (lifetime.contains(depLevel))
									{
										auto& entry = lifetime.at(depLevel);
										if (entry.InputLayout != layout)
										{
											ZE_CHECK_FAILED_GRAPH_COMPUTE(mergeReadOnlyLayout(entry.InputLayout, layout), ErrorResourceInputLayoutMismatch,
												"Input resource [" + std::string(name) + "] of pass [" + renderNode.GetFullName() + "] at dependency level " +
												std::to_string(depLevel) + " is being used in different layouts at the same time!");
										}
										entry.PossibleAccess |= GetAccessFromLayout(layout);
										entry.WorkGfx |= renderNode.IsGfxPass();
										entry.WorkCompute |= renderNode.IsComputePass();
										entry.WorkRayTracing |= renderNode.IsRayTracingPass();
										entry.AsyncQueue |= async;
									}
									else
									{
										lifetime.emplace(depLevel, ResourceState{ layout, TextureLayout::Undefined,
											GetAccessFromLayout(layout), renderNode.IsGfxPass(), renderNode.IsComputePass(),
											renderNode.IsRayTracingPass(), async, execGroupIndex, j });
									}
								}
							}

							for (ResIndex output = 0; output < computed.OutputResources.size(); ++output)
							{
								std::string_view name = computed.OutputResources.at(output);

								if (name != "")
								{
									auto& lifetime = resourceLifetimes.at(resourceLookup.at(name));
									TextureLayout layout = renderNode.GetOutputLayout(output);

									if (lifetime.contains(depLevel))
									{
										auto& entry = lifetime.at(depLevel);
										if (entry.OutputLayout == TextureLayout::Undefined)
											entry.OutputLayout = layout;
										else if (entry.OutputLayout != layout)
										{
											ZE_CHECK_FAILED_GRAPH_COMPUTE(mergeReadOnlyLayout(entry.OutputLayout, layout), ErrorResourceOutputLayoutMismatch,
												"Resource [" + std::string(name) + "] outputted by pass [" + renderNode.GetFullName() + "] at dependency level " +
												std::to_string(depLevel) + " is being used in different layouts at the same time!");
										}
										entry.PossibleAccess |= GetAccessFromLayout(layout);
										entry.WorkGfx |= renderNode.IsGfxPass();
										entry.WorkCompute |= renderNode.IsComputePass();
										entry.WorkRayTracing |= renderNode.IsRayTracingPass();
										entry.AsyncQueue |= async;
									}
									else
									{
										lifetime.emplace(depLevel, ResourceState{ TextureLayout::Undefined, layout,
											GetAccessFromLayout(layout), renderNode.IsGfxPass(), renderNode.IsComputePass(),
											renderNode.IsRayTracingPass(), async, execGroupIndex, j });
									}
								}
							}

							for (ResIndex inner = 0; inner < renderNode.GetInnerBuffers().size(); ++inner)
							{
								auto& lifetime = resourceLifetimes.at(resourceLookup.at(renderNode.GetInnerBufferName(inner)));
								TextureLayout layout = renderNode.GetInnerBufferLayout(inner);

								lifetime.emplace(depLevel, ResourceState{ layout, layout,
									GetAccessFromLayout(layout), renderNode.IsGfxPass(),
									renderNode.IsComputePass(), renderNode.IsRayTracingPass(),
									async, execGroupIndex, j });
							}
						}
					}
					return BuildResult::Success;
				};
			for (U32 i = 0; i < graph.execGroupCount; ++i)
			{
				BuildResult result = gatherResourceUsage(graph.passExecGroups[i].at(0), i, false);
				if (result != BuildResult::Success)
					return result;
				if (asyncComputeEnabled)
				{
					result = gatherResourceUsage(graph.passExecGroups[i].at(1), i, true);
					if (result != BuildResult::Success)
						return result;
				}
			}
		}

		// Compute what layout changes are needed between resource usages
		{
			ZE_PERF_GUARD("RenderGraphBuilder::FillPassBarriers - compute transitions");

			for (RID rid = 0; rid < resourceLifetimes.size(); ++rid)
			{
				auto& lifetime = resourceLifetimes.at(rid);
				ZE_ASSERT(lifetime.size() > 0, "Computing transitions for resource that have no record of usage!");

				// Fix for in/out layouts when resource is used first time or not exposed later directly
				for (auto& state : lifetime)
				{
					if (state.second.InputLayout == TextureLayout::Undefined)
						state.second.InputLayout = state.second.OutputLayout;
					if (state.second.OutputLayout == TextureLayout::Undefined)
						state.second.OutputLayout = state.second.InputLayout;
				}

				auto placeTransition = [&](std::vector<BarrierTransition>& beginBarriers,
					std::vector<BarrierTransition>& endBarriers, BarrierTransition& barrier, bool noSplitUseEnd)
					{
						if (noSplitUseEnd || (flags & GraphFinalizeFlag::NoSplitBarriersUseEnd))
							endBarriers.emplace_back(barrier);
						else if (flags & GraphFinalizeFlag::NoSplitBarriersUseBegin)
							beginBarriers.emplace_back(barrier);
						else
						{
							barrier.Type = BarrierType::SplitBegin;
							beginBarriers.emplace_back(barrier);
							barrier.Type = BarrierType::SplitEnd;
							endBarriers.emplace_back(barrier);
						}
					};
				for (auto it = lifetime.begin(); it != lifetime.end(); ++it)
				{
					auto next = it;
					++next;
					if (next != lifetime.end())
					{
						auto& begin = it->second;
						auto& end = next->second;
						if (begin.OutputLayout != end.InputLayout)
						{
							ZE_CHECK_FAILED_GRAPH_COMPUTE((begin.ExecGroupIndex == end.ExecGroupIndex && (begin.PassGroupIndex >= end.PassGroupIndex || begin.AsyncQueue != end.AsyncQueue))
								|| begin.ExecGroupIndex > end.ExecGroupIndex, ErrorResourceLayoutChangesInIncorrectOrder,
								"Layout of the resource [" + std::string(computedResources.at(rid)) + "] changes between incorrect execution groups or pass groups!");

							BarrierTransition barrier =
							{
								rid, begin.OutputLayout, end.InputLayout,
								begin.PossibleAccess, end.PossibleAccess,
								GetSyncFromAccess(begin.PossibleAccess, begin.WorkGfx, begin.WorkCompute, begin.WorkRayTracing),
								GetSyncFromAccess(end.PossibleAccess, end.WorkGfx, end.WorkCompute, end.WorkRayTracing),
								BarrierType::Immediate
							};
							auto& endExecGroup = end.GetExecGroup(graph);
							ZE_ASSERT(endExecGroup.PassGroupCount, "Placing barrier in execution group without any passes!");

							if (begin.ExecGroupIndex == end.ExecGroupIndex)
							{
								placeTransition(endExecGroup.PassGroups[begin.PassGroupIndex + 1].StartBarriers,
									end.GetPassGroup(graph, endExecGroup).StartBarriers, barrier,
									begin.PassGroupIndex + 1 == end.PassGroupIndex);
							}
							else
							{
								auto& beginExecGroup = begin.GetExecGroup(graph);
								ZE_ASSERT(beginExecGroup.PassGroupCount, "Placing barrier in execution group without any passes!");
								// Move barrier to the end barriers section
								const bool lastGroup = begin.PassGroupIndex + 1 == beginExecGroup.PassGroupCount;

								// Different queues, have to execute transition on starting queue
								if (begin.AsyncQueue != end.AsyncQueue)
								{
									placeTransition(beginExecGroup.PassGroups[lastGroup ? 0 : begin.PassGroupIndex + 1].StartBarriers,
										beginExecGroup.EndBarriers, barrier, lastGroup);
								}
								else
								{
									// Last pass group in exec group, prefer using end exec group for possibility of more barriers in one place
									if (lastGroup)
									{
										placeTransition(endExecGroup.PassGroups[0].StartBarriers,
											end.GetPassGroup(graph, endExecGroup).StartBarriers, barrier, end.PassGroupIndex == 0);
									}
									else if (flags & GraphFinalizeFlag::NoSplitBarriersUseBegin)
										beginExecGroup.PassGroups[begin.PassGroupIndex + 1].StartBarriers.emplace_back(barrier);
									else if (flags & GraphFinalizeFlag::NoSplitBarriersUseEnd)
										end.GetPassGroup(graph, endExecGroup).StartBarriers.emplace_back(barrier);
									else
									{
										if (flags & GraphFinalizeFlag::CrossExecGroupSplitBarriersUseEndGroup)
										{
											if (end.PassGroupIndex != 0)
											{
												barrier.Type = BarrierType::SplitBegin;
												endExecGroup.PassGroups[0].StartBarriers.emplace_back(barrier);
												barrier.Type = BarrierType::SplitEnd;
											}
											end.GetPassGroup(graph, endExecGroup).StartBarriers.emplace_back(barrier);
										}
										else
										{
											barrier.Type = BarrierType::SplitBegin;
											beginExecGroup.PassGroups[begin.PassGroupIndex + 1].StartBarriers.emplace_back(barrier);
											barrier.Type = BarrierType::SplitEnd;
											beginExecGroup.EndBarriers.emplace_back(barrier);
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// Intial transitions for every resource + end wrapping transitions when resource is temporal
		ZE_PERF_START("RenderGraphBuilder::FillPassBarriers - initial transitions");
		for (RID rid = 0; rid < resourceLifetimes.size(); ++rid)
		{
			auto& lifetime = resourceLifetimes.at(rid);
			auto firstUsage = lifetime.begin();
			auto lastUsage = lifetime.end();
			ZE_ASSERT(firstUsage->second.GetExecGroup(graph).PassGroupCount, "Placing barrier in execution group without any passes!");

			if (resources.Get(std::string(computedResources.at(rid))).Flags & FrameResourceFlag::Temporal)
			{
				TextureLayout firstLayout = firstUsage->second.InputLayout;
				TextureLayout lastLayout = lastUsage->second.OutputLayout;
				if (firstLayout != lastLayout)
				{
					ZE_ASSERT(lastUsage->second.GetExecGroup(graph).PassGroupCount, "Placing barrier in execution group without any passes!");

					BarrierTransition barrier =
					{
						rid, lastLayout, firstLayout,
						lastUsage->second.PossibleAccess, firstUsage->second.PossibleAccess,
						GetSyncFromAccess(lastUsage->second.PossibleAccess, lastUsage->second.WorkGfx, lastUsage->second.WorkCompute, lastUsage->second.WorkRayTracing),
						GetSyncFromAccess(firstUsage->second.PossibleAccess, firstUsage->second.WorkGfx, firstUsage->second.WorkCompute, firstUsage->second.WorkRayTracing),
						BarrierType::Immediate
					};

					// Everything is reversed in comparison to normal resources
					if (flags & GraphFinalizeFlag::InitializeResourcesWhereMostBarriers)
					{
						U32 maxCount = 0;
						U32 maxExecGroupIndex = 0, maxPassGroupIndex = 0;
						bool useEndBarriers = false;
						for (U32 index = graph.execGroupCount, first = lastUsage->second.ExecGroupIndex; index > first;)
						{
							auto& group = graph.passExecGroups[--index].at(static_cast<U64>(lastUsage->second.AsyncQueue));
							if (group.PassGroupCount > 0)
							{
								if (index != first && Utils::SafeCast<U32>(group.EndBarriers.size()) >= maxCount)
								{
									maxCount = Utils::SafeCast<U32>(group.EndBarriers.size());
									maxExecGroupIndex = index;
									maxPassGroupIndex = 0;
									useEndBarriers = true;
								}

								// If firstt group then move only down to the target pass
								for (U32 i = group.PassGroupCount, firstPass = index == first ? lastUsage->second.PassGroupIndex + 1 : 0; i > firstPass;)
								{
									// Greater or equal since moving closer to final pass will only benefit barriers
									if (Utils::SafeCast<U32>(group.PassGroups[--i].StartBarriers.size()) >= maxCount)
									{
										maxCount = Utils::SafeCast<U32>(group.PassGroups[i].StartBarriers.size());
										maxExecGroupIndex = index;
										maxPassGroupIndex = i;
										useEndBarriers = false;
									}
								}
							}
						}
						if (maxCount == 0)
						{
							auto& group = lastUsage->second.GetExecGroup(graph);
							if (lastUsage->second.PassGroupIndex + 1 == group.PassGroupCount)
								group.EndBarriers.emplace_back(barrier);
							else
								group.PassGroups[lastUsage->second.PassGroupIndex + 1].StartBarriers.emplace_back(barrier);
						}
						else if (useEndBarriers)
							graph.passExecGroups[maxExecGroupIndex].at(static_cast<U64>(lastUsage->second.AsyncQueue)).EndBarriers.emplace_back(barrier);
						else
							graph.passExecGroups[maxExecGroupIndex].at(static_cast<U64>(lastUsage->second.AsyncQueue)).PassGroups[maxPassGroupIndex].StartBarriers.emplace_back(barrier);
					}
					else if (flags & GraphFinalizeFlag::InitializeResourcesBeforePass)
					{
						auto& group = lastUsage->second.GetExecGroup(graph);
						if (lastUsage->second.PassGroupIndex + 1 == group.PassGroupCount)
							group.EndBarriers.emplace_back(barrier);
						else
							group.PassGroups[lastUsage->second.PassGroupIndex + 1].StartBarriers.emplace_back(barrier);
					}
					else if (flags & GraphFinalizeFlag::InitializeResourcesSplitBarrier)
					{
						auto& group = lastUsage->second.GetExecGroup(graph);
						if (lastUsage->second.PassGroupIndex + 1 != group.PassGroupCount)
						{
							barrier.Type = BarrierType::SplitBegin;
							group.PassGroups[lastUsage->second.PassGroupIndex + 1].StartBarriers.emplace_back(barrier);
							barrier.Type = BarrierType::SplitEnd;
						}
						group.EndBarriers.emplace_back(barrier);
					}
					else // InitializeResourcesFrameStart
					{
						// Check for further exec groups to have any passes to perform work on given queue
						bool notPlaced = true;
						for (U32 index = graph.execGroupCount - 1, first = lastUsage->second.ExecGroupIndex; index > first; --index)
						{
							auto& group = graph.passExecGroups[index].at(static_cast<U64>(lastUsage->second.AsyncQueue));
							if (group.PassGroupCount > 0)
							{
								group.EndBarriers.emplace_back(barrier);
								notPlaced = false;
								break;
							}
						}
						if (notPlaced)
							lastUsage->second.GetExecGroup(graph).EndBarriers.emplace_back(barrier);
					}
				}
			}
			else
			{
				BarrierTransition barrier =
				{
					rid, TextureLayout::Undefined, firstUsage->second.InputLayout,
					Base(ResourceAccess::None), firstUsage->second.PossibleAccess,
					Base(StageSync::None),
					GetSyncFromAccess(firstUsage->second.PossibleAccess, firstUsage->second.WorkGfx, firstUsage->second.WorkCompute, firstUsage->second.WorkRayTracing),
					BarrierType::Immediate
				};

				if (flags & GraphFinalizeFlag::InitializeResourcesWhereMostBarriers)
				{
					U32 maxCount = 0;
					U32 maxExecGroupIndex = 0, maxPassGroupIndex = 0;
					bool useEndBarriers = false;
					for (U32 index = 0, last = firstUsage->second.ExecGroupIndex; index <= last; ++index)
					{
						auto& group = graph.passExecGroups[index].at(static_cast<U64>(firstUsage->second.AsyncQueue));
						if (group.PassGroupCount > 0)
						{
							// If last group then move only up to the target pass
							for (U32 i = 0, size = index == last ? firstUsage->second.PassGroupIndex + 1 : group.PassGroupCount; i < size; ++i)
							{
								// Greater or equal since moving closer to final pass will only benefit barriers
								if (Utils::SafeCast<U32>(group.PassGroups[i].StartBarriers.size()) >= maxCount)
								{
									maxCount = Utils::SafeCast<U32>(group.PassGroups[i].StartBarriers.size());
									maxExecGroupIndex = index;
									maxPassGroupIndex = i;
									useEndBarriers = false;
								}
							}
							if (index != last && Utils::SafeCast<U32>(group.EndBarriers.size()) >= maxCount)
							{
								maxCount = Utils::SafeCast<U32>(group.EndBarriers.size());
								maxExecGroupIndex = index;
								maxPassGroupIndex = 0;
								useEndBarriers = true;
							}
						}
					}
					if (maxCount == 0)
						firstUsage->second.GetPassGroup(graph).StartBarriers.emplace_back(barrier);
					else if (useEndBarriers)
						graph.passExecGroups[maxExecGroupIndex].at(static_cast<U64>(firstUsage->second.AsyncQueue)).EndBarriers.emplace_back(barrier);
					else
						graph.passExecGroups[maxExecGroupIndex].at(static_cast<U64>(firstUsage->second.AsyncQueue)).PassGroups[maxPassGroupIndex].StartBarriers.emplace_back(barrier);
				}
				else if (flags & GraphFinalizeFlag::InitializeResourcesBeforePass)
					firstUsage->second.GetPassGroup(graph).StartBarriers.emplace_back(barrier);
				else if (flags & GraphFinalizeFlag::InitializeResourcesSplitBarrier)
				{
					if (firstUsage->second.PassGroupIndex != 0)
					{
						barrier.Type = BarrierType::SplitBegin;
						firstUsage->second.GetExecGroup(graph).PassGroups[0].StartBarriers.emplace_back(barrier);
						barrier.Type = BarrierType::SplitEnd;
					}
					firstUsage->second.GetPassGroup(graph).StartBarriers.emplace_back(barrier);
				}
				else // InitializeResourcesFrameStart
				{
					// Check for previous exec groups to have any passes to perform work on given queue
					bool notPlaced = true;
					for (U32 index = 0, last = firstUsage->second.ExecGroupIndex; index < last; ++index)
					{
						auto& group = graph.passExecGroups[index].at(static_cast<U64>(firstUsage->second.AsyncQueue));
						if (group.PassGroupCount > 0)
						{
							group.PassGroups[0].StartBarriers.emplace_back(barrier);
							notPlaced = false;
							break;
						}
					}
					if (notPlaced)
						firstUsage->second.GetExecGroup(graph).PassGroups[0].StartBarriers.emplace_back(barrier);
				}
			}
		}
		ZE_PERF_STOP();

		// Insert transition to the present state at the end of the graph if required
		if (!(flags & GraphFinalizeFlag::NoPresentBarrier))
		{
			auto& lastUsage = resourceLifetimes.at(BACKBUFFER_RID).rbegin()->second;
			TextureLayout lastLayout = lastUsage.OutputLayout;
			if (lastLayout != TextureLayout::Present)
			{
				BarrierTransition barrier =
				{
					BACKBUFFER_RID, lastLayout, TextureLayout::Present,
					lastUsage.PossibleAccess, Base(ResourceAccess::Common),
					GetSyncFromAccess(lastUsage.PossibleAccess, lastUsage.WorkGfx, lastUsage.WorkCompute, lastUsage.WorkRayTracing),
					Base(StageSync::AllGraphics),
					BarrierType::Immediate
				};
				lastUsage.GetExecGroup(graph).EndBarriers.emplace_back(barrier);
			}
		}

		return BuildResult::Success;
	}

	BuildResult RenderGraphBuilder::LoadConfig(Device& dev, const RenderGraphDesc& desc) noexcept
	{
		ZE_PERF_GUARD("RenderGraphBuilder::LoadConfig");

		// Clear previous config on start for sanity
		ClearConfig(dev);

		// Can be run on multiple threads possibly
		BuildResult result = LoadGraphDesc(dev, desc);
		if (result != BuildResult::Success)
		{
			ClearConfig(dev);
			return result;
		}

		result = LoadResourcesDesc(dev, desc);
		if (result != BuildResult::Success)
		{
			ClearConfig(dev);
			return result;
		}
		return BuildResult::Success;
	}

	BuildResult RenderGraphBuilder::ComputeGraph(Device& dev, bool minimizeDistances) noexcept
	{
		// Get master list of resources (remove the ones that are not referenced by any pass in full configuration
		// and issue warning) - DONE
		//
		// After that create graph of nodes. Merge the ones that have same graph connector name so they
		// will occupy same place but will be chosen based on current configuration. - DONE
		//
		// Compute as much as possible sync points between queues, order passes into execution groups,
		// reorder them based on hints, etc.
		//
		// Later on during graph building there will be more computing based on current actual configuration,
		// each pass will be checked if it can run in current setup, process nodes will be culled if no input,
		// producers will be present, etc. - DONE

		ZE_CHECK_FAILED_GRAPH_COMPUTE(!passDescs.size() || !resources.Size()
			|| passDescs.size() != renderGraphDepList.size() || passDescs.size() != topoplogyOrder.size(),
			ErrorConfigNotLoaded, "Computing render graph while no config has been properly loaded!");

		ZE_PERF_GUARD("RenderGraphBuilder::ComputeGraph");
		ClearComputedGraph(dev);

		// Check for presence of nodes in current configuration, first by evaluation value
		std::vector<PresenceInfo> presentNodes(passDescs.size());
		{
			ZE_PERF_GUARD("RenderGraphBuilder::ComputeGraph - evaluate nodes");
			for (U32 i = 0; i < passDescs.size(); ++i)
			{
				const auto& passGroup = passDescs.at(i);
				for (U32 j = 0; j < passGroup.size(); ++j)
				{
					const auto& pass = passGroup.at(j);
					if (pass.GetExecType() == PassExecutionType::DynamicProcessor
						|| pass.GetDesc().Evaluate == nullptr || pass.GetDesc().Evaluate())
					{
						auto& node = presentNodes.at(i);

						ZE_CHECK_FAILED_GRAPH_COMPUTE(node.Present, ErrorMultiplePresentPassesWithSameConnectorName,
							"Found multiple passes in same connector group that are present at the same time! Wrong passes: [" +
							passDescs.at(i).at(node.NodeGroupIndex).GetFullName() + "], [" + passDescs.at(i).at(j).GetFullName() + "].");

						node.Present = true;
						node.ActiveInputProducerPresent = node.ProducerChecked = pass.GetExecType() == PassExecutionType::Producer;
						node.NodeGroupIndex = j;
					}
				}
			}
		}

		// Create adjacency graph for current configuration from dependency lists
		ZE_PERF_START("RenderGraphBuilder::ComputeGraph - create adjacency graph");
		std::vector<std::vector<U32>> graphList(passDescs.size());
		for (U32 i = 0; i < graphList.size(); ++i)
		{
			for (const auto& prevNode : renderGraphDepList.at(i).NodesDependecies.at(presentNodes.at(i).NodeGroupIndex).PreceedingNodes)
			{
				graphList.at(prevNode.NodeIndex).emplace_back(i);
			}
		}
		ZE_PERF_STOP();

		// Check present processor nodes if they have all required input from producers and if their output is consumed by some producers
		ZE_PERF_START("RenderGraphBuilder::ComputeGraph - compute graph culling");
		for (U32 i = 0; i < presentNodes.size(); ++i)
		{
			auto& node = presentNodes.at(i);
			if (node.Present)
			{
				if (!node.ProducerChecked)
					CheckNodeProducerPresence(i, presentNodes);
				if (!node.ConsumerChecked)
					CheckNodeConsumerPresence(i, presentNodes, graphList);
			}
		}
		ZE_PERF_STOP();

		// Only passes with both input and outputs will be present in the computed graph
		ZE_PERF_START("RenderGraphBuilder::ComputeGraph - cull graph");
		computedGraph.resize(passDescs.size());
		for (U32 i = 0; i < computedGraph.size(); ++i)
		{
			const auto& presence = presentNodes.at(i);
			auto& computedNode = computedGraph.at(i);

			computedNode.Present = presence.Present;
			computedNode.NodeGroupIndex = presence.NodeGroupIndex;
			if (passDescs.at(i).at(presence.NodeGroupIndex).GetExecType() != PassExecutionType::Producer)
				computedNode.Present = computedNode.Present && presence.ActiveInputProducerPresent && presence.ActiveOutputProducerPresent;
		}
		presentNodes.clear();
		ZE_PERF_STOP();

		// Fill real in/out resources and dependency levels
		{
			ZE_PERF_GUARD("RenderGraphBuilder::ComputeGraph - fill resources");

			dependencyLevels.resize(passDescs.size(), 0);
			dependencyLevelCount = 0;
			for (U32 node : topoplogyOrder)
			{
				auto& computed = computedGraph.at(node);
				const auto& renderNode = passDescs.at(node).at(computed.NodeGroupIndex);
				const auto& dependencies = renderGraphDepList.at(node).NodesDependecies.at(computed.NodeGroupIndex).PreceedingNodes;

				std::vector<std::string> originalInputs;
				originalInputs.reserve(renderNode.GetInputs().size());
				computed.InputResources.reserve(renderNode.GetInputs().size());

				// Find all input resources in current configuration
				for (ResIndex i = 0, size = Utils::SafeCast<ResIndex>(renderNode.GetInputs().size()); i < size; ++i)
				{
					const auto& input = renderNode.GetInputs().at(i);

					// Get name of the input pass and find it
					std::deque<std::string_view> split = Utils::SplitString(input, ".");
					ZE_CHECK_FAILED_GRAPH_COMPUTE(split.size() != 2, ErrorPassInputIncorrectFormat,
						"Input of pass [" + renderNode.GetFullName() + "] is in incorrect format [" + input + "]!");

					auto it = std::find_if(dependencies.begin(), dependencies.end(), [&](const GraphConnection& dep)
						{
							return passDescs.at(dep.NodeIndex).at(computedGraph.at(dep.NodeIndex).NodeGroupIndex).GetGraphConnectorName() == split.at(0);
						});
					ZE_CHECK_FAILED_GRAPH_COMPUTE(it == dependencies.end(), ErrorPassNameNotFound,
						"Cannot find dependency [" + std::string(split.at(0)) + "] of pass [" + renderNode.GetFullName() + "]!");

					// Check which output matches current input
					const auto& prevComputed = computedGraph.at(it->NodeIndex);
					const auto& prevNode = passDescs.at(it->NodeIndex).at(prevComputed.NodeGroupIndex);
					for (ResIndex j = 0, outSize = Utils::SafeCast<ResIndex>(prevNode.GetOutputs().size()); j < outSize; ++j)
					{
						if (prevNode.GetOutputs().at(j) == input)
						{
							originalInputs.emplace_back(prevNode.GetOutputResources().at(j));
							computed.InputResources.emplace_back(prevComputed.OutputResources.at(j));

							ZE_CHECK_FAILED_GRAPH_COMPUTE(computed.Present && computed.InputResources.back() == "" && renderNode.IsInputRequired(i),
								ErrorMissingNonOptionalInput, "Input [" + input + "] of pass [" + renderNode.GetFullName() + "] is missing it's resource!");
							break;
						}
					}
				}
				ZE_CHECK_FAILED_GRAPH_COMPUTE(originalInputs.size() != renderNode.GetInputs().size(), ErrorNotAllInputsFound,
					"Cannot find al inputs for pass [" + renderNode.GetFullName() + "]!");

				// Determine which output resources will be present
				computed.OutputResources.reserve(renderNode.GetOutputResources().size());
				for (ResIndex i = 0, size = Utils::SafeCast<ResIndex>(renderNode.GetOutputResources().size()); i < size; ++i)
				{
					const auto& output = renderNode.GetOutputResources().at(i);
					const auto& replacement = renderNode.GetOutputReplacementResources().at(i);

					auto it = std::find(originalInputs.begin(), originalInputs.end(), output);
					if (computed.Present)
					{
						if (it != originalInputs.end())
						{
							computed.OutputResources.emplace_back(computed.InputResources.at(std::distance(originalInputs.begin(), it)));
						}
						else
						{
							computed.OutputResources.emplace_back(output);
						}
					}
					else if (replacement != "")
					{
						computed.OutputResources.emplace_back(replacement);
					}
					else if (it != originalInputs.end())
					{
						computed.OutputResources.emplace_back(computed.InputResources.at(std::distance(originalInputs.begin(), it)));
					}
					else
					{
						computed.OutputResources.emplace_back();
					}
				}

				// Compute longest path for each node as it's dependency level
				U32 nodeLevel = dependencyLevels.at(node);
				for (U32 adjNode : graphList.at(node))
				{
					if (dependencyLevels.at(adjNode) <= nodeLevel)
						dependencyLevels.at(adjNode) = nodeLevel + 1;
					if (dependencyLevels.at(adjNode) > dependencyLevelCount)
						dependencyLevelCount = dependencyLevels.at(adjNode);
				}
			}
			++dependencyLevelCount;
		}

		// Minimize distances between nodes when possible
		if (minimizeDistances)
		{
			ZE_PERF_GUARD("RenderGraphBuilder::ComputeGraph - minimize distances");
			for (auto& list : graphList)
				list.clear();
			for (U32 i = 0; i < renderGraphDepList.size(); ++i)
			{
				const auto& deps = renderGraphDepList.at(i).NodesDependecies.at(computedGraph.at(i).NodeGroupIndex).PreceedingNodes;
				U32 currentLevel = dependencyLevels.at(i);
				for (const auto& dep : deps)
					graphList.at(dep.NodeIndex).emplace_back(currentLevel);
			}
			for (U32 i = 0; i < graphList.size(); ++i)
			{
				auto& list = graphList.at(i);
				if (list.size() > 0)
				{
					std::sort(list.begin(), list.end());
					dependencyLevels.at(i) = list.front() - 1;
				}
			}
		}
		graphList.clear();

		// Mark active resources with correct flags
		ZE_PERF_START("RenderGraphBuilder::ComputeGraph - get resources flags");
		auto getFlagsFromLayout = [](TextureLayout layout) -> FrameResourceFlags
			{
				FrameResourceFlags flags = Base(FrameResourceFlag::InternalResourceActive);
				switch (layout)
				{
				case ZE::GFX::Pipeline::TextureLayout::RenderTarget:
					flags |= FrameResourceFlag::InternalUsageRenderTarget;
					break;
				case ZE::GFX::Pipeline::TextureLayout::UnorderedAccess:
					flags |= FrameResourceFlag::InternalUsageUnorderedAccess;
					break;
				case ZE::GFX::Pipeline::TextureLayout::DepthStencilWrite:
				case ZE::GFX::Pipeline::TextureLayout::DepthStencilRead:
					flags |= FrameResourceFlag::InternalUsageDepth;
					break;
				case ZE::GFX::Pipeline::TextureLayout::Common:
				case ZE::GFX::Pipeline::TextureLayout::GenericRead:
				case ZE::GFX::Pipeline::TextureLayout::ShaderResource:
					flags |= FrameResourceFlag::InternalUsageShaderResource;
					break;
				default:
					break;
				}
				return flags;
			};
		for (U32 i = 0; i < computedGraph.size(); ++i)
		{
			const auto& computed = computedGraph.at(i);
			if (computed.Present)
			{
				const auto& renderNode = passDescs.at(i).at(computed.NodeGroupIndex);

				// Check for async compute for possibility to skip computation of sync points later
				asyncComputeEnabled |= renderNode.IsAsync();

				for (ResIndex j = 0, size = Utils::SafeCast<ResIndex>(computed.InputResources.size()); j < size; ++j)
				{
					const auto& res = computed.InputResources.at(j);
					if (res != "")
						resources.Get(res).Flags |= getFlagsFromLayout(renderNode.GetInputLayout(j));
				}
				for (ResIndex j = 0, size = Utils::SafeCast<ResIndex>(renderNode.GetInnerBuffers().size()); j < size; ++j)
				{
					resources.Get(renderNode.GetInnerBufferName(j)).Flags |= getFlagsFromLayout(renderNode.GetInnerBufferLayout(j));
				}
				for (ResIndex j = 0, size = Utils::SafeCast<ResIndex>(computed.OutputResources.size()); j < size; ++j)
				{
					const auto& res = computed.OutputResources.at(j);
					if (res != "")
						resources.Get(res).Flags |= getFlagsFromLayout(renderNode.GetOutputLayout(j));
				}
			}
		}
		ZE_PERF_STOP();

		ZE_PERF_START("RenderGraphBuilder::ComputeGraph - check correct resources flags");
		BuildResult result = BuildResult::Success;
		resources.TransformCheck([&result](const std::string& name, const FrameResourceDesc& res) -> bool
			{
				if ((res.Flags & (FrameResourceFlag::InternalUsageRenderTarget | FrameResourceFlag::InternalUsageUnorderedAccess))
					&& (res.Flags & FrameResourceFlag::InternalUsageDepth))
				{
					ZE_FAIL("Cannot create depth stencil together with render target or unordered access view for same resource [" + name + "]!");
					result = BuildResult::ErrorIncorrectResourceUsage;
					return true;
				}
				if ((res.Flags & FrameResourceFlag::InternalUsageRenderTarget) && Utils::IsDepthStencilFormat(res.Format))
				{
					ZE_FAIL("Cannot use depth stencil format with render target for resource [" + name + "]!");
					result = BuildResult::ErrorIncorrectResourceFormat;
					return true;
				}
				if (res.Type != FrameResourceType::Texture2D && res.Type != FrameResourceType::TextureCube && (res.Flags & FrameResourceFlag::InternalUsageDepth))
				{
					ZE_FAIL("Cannot create non-2D or cube texture as depth stencil in resource [" + name + "]!");
					result = BuildResult::ErrorWrongResourceConfiguration;
					return true;
				}
				if ((res.Flags & FrameResourceFlag::SimultaneousAccess) && (res.Flags & FrameResourceFlag::InternalUsageDepth))
				{
					ZE_FAIL("Simultaneous access cannot be used on depth stencil in resource [" + name + "]!");
					result = BuildResult::ErrorWrongResourceConfiguration;
					return true;
				}
				return false;
			});
		ZE_PERF_STOP();

		if (result != BuildResult::Success)
		{
			ClearComputedGraph(dev);
			return result;
		}

		// Get final list of resources
		ZE_PERF_START("RenderGraphBuilder::ComputeGraph - set final resources list");
		computedResources.emplace_back(BACKBUFFER_NAME);
		resources.Transform([&](const std::string& name, const FrameResourceDesc& res)
			{
				if ((res.Flags & FrameResourceFlag::InternalResourceActive) && name != BACKBUFFER_NAME)
					computedResources.emplace_back(name);
			});
		ZE_PERF_STOP();

		return BuildResult::Success;
	}

	BuildResult RenderGraphBuilder::FinalizeGraph(Graphics& gfx, Data::AssetsStreamer& assets, RenderGraph& graph, const RenderGraphDesc& desc, GraphFinalizeFlags flags)
	{
		Device& dev = gfx.GetDevice();

		// In case that graph have not been yet computed
		if (!IsGraphComputed())
		{
			ZE_WARNING("Graph needs to be computed before finalizing it!");
			BuildResult result = ComputeGraph(dev);
			if (result != BuildResult::Success)
				return result;
		}
		ZE_PERF_GUARD("RenderGraphBuilder::FinalizeGraph");

		graph.dynamicBuffers.Exec([&dev](auto& buffer) { buffer.Init(dev); });
		graph.execData.CustomData = desc.PassCustomData;
		graph.execData.SettingsData = desc.SettingsData;

		// Send to GPU new graph data
		Resource::CBufferData settingsData = {};
		settingsData.DataStatic = &graph.execData.SettingsData;
		settingsData.Bytes = sizeof(RendererSettingsData);
		graph.execData.SettingsBuffer.Init(dev, assets.GetDisk(), settingsData);

		// Initialize passes structure
		GroupRenderPasses(dev, graph);

		// Need proper interface before passes will start using it
		graph.ffxInterface = FFX::GetInterface(dev, graph.dynamicBuffers, graph.execData.Buffers, assets.GetDisk(), graph.ffxInternalBuffers, graph.ffxBuffersChanged);

		// Perform all needed work for active passes
		InitializeRenderPasses(dev, assets, graph, desc);

		// After render passes has been initialized, new frame buffer can be created with all new setttings applied
		graph.execData.Buffers.Init(dev, GetFrameBufferLayout(graph));

		// Check for sync dependencies between execution groups
		ComputeGroupSyncs(graph);

		// Skip computation of barriers where not required
		if (Settings::GetGfxApi() != GfxApiType::DX11 && Settings::GetGfxApi() != GfxApiType::OpenGL)
			return FillPassBarriers(dev, graph, flags);

		return BuildResult::Success;
	}

	void RenderGraphBuilder::ClearConfig(Device& dev) noexcept
	{
		resourceOptions = Base(FrameBufferFlag::None);
		resources.Clear();
		passDescs.clear();
		renderGraphDepList.clear();
		topoplogyOrder.clear();

		ClearComputedGraph(dev);
	}

	void RenderGraphBuilder::ClearComputedGraph(Device& dev) noexcept
	{
		execDataCache.Transform([&dev](std::pair<PtrVoid, PassCleanCallback>& execData)
			{
				if (execData.first)
				{
					ZE_ASSERT(execData.second, "Clean function should always be present when exec data is not empty!");
					execData.second(dev, execData.first);
				}
			});

		execDataCache.Clear();
		computedGraph.clear();
		dependencyLevels.clear();
		computedResources.clear();
		asyncComputeEnabled = false;
		dependencyLevelCount = 0;
		resources.Transform([](FrameResourceDesc& desc) { desc.Flags &= ~FrameResourceFlag::InternalFlagsMask; });
	}
}