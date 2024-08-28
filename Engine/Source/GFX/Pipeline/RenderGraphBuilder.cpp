#include "GFX/Pipeline/RenderGraphBuilder.h"
#include "GFX/Pipeline/RenderGraph.h"

// Helper macro to end loading config and return when condition is true
#define ZE_CHECK_FAILED_CONFIG_LOAD(condition, result, message) do { if (condition) { ZE_FAIL(message); ClearConfig(); return BuildResult::##result; } } while (false)
// Helper macro to end computing graph and return when condition is true
#define ZE_CHECK_FAILED_GRAPH_COMPUTE(condition, result, message) do { if (condition) { ZE_FAIL(message); ClearComputedGraph(); return BuildResult::##result; } } while (false)

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

	BuildResult RenderGraphBuilder::LoadGraphDesc(const RenderGraphDesc& desc) noexcept
	{
		ZE_PERF_GUARD("RenderGraphBuilder::LoadGraphDesc");

		ZE_CHECK_FAILED_CONFIG_LOAD(desc.RenderPasses.size() != Utils::SafeCast<U32>(desc.RenderPasses.size()), ErrorTooManyPasses,
			"Number of passes cannot exceed UINT32_MAX!");

		// Gather passes and group them by graph connector names
		for (U32 i = 0; i < desc.RenderPasses.size(); ++i)
		{
			const RenderNode& node = desc.RenderPasses.at(i);

			ZE_CHECK_FAILED_CONFIG_LOAD(!node.GetDesc().Execute, ErrorPassExecutionCallbackNotProvided,
				"Execution callback missing in [" + node.GetFullName() + "]!");
			ZE_CHECK_FAILED_CONFIG_LOAD(node.GetDesc().InitData && !node.GetDesc().FreeInitData, ErrorPassFreeInitDataCallbackNotProvided,
				"FreeInitData callback missing in [" + node.GetFullName() + "]!");
			ZE_CHECK_FAILED_CONFIG_LOAD(node.GetDesc().InitData && !node.GetDesc().Init, ErrorPassFreeInitDataCallbackNotProvided,
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

		// Init dependency info for all nodes in given group
		renderGraphDepList.resize(passDescs.size());
		for (U32 i = 0; i < passDescs.size(); ++i)
			renderGraphDepList.at(i).NodesDependecies.resize(passDescs.at(i).size());

		// Create graph via reversed adjacency list (list of node groups and for each node in a group
		// list of nodes from which traversal is possible as data flow)
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

		// Sort nodes in topological order
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
		return BuildResult::Success;
	}

	BuildResult RenderGraphBuilder::LoadResourcesDesc(const RenderGraphDesc& desc) noexcept
	{
		ZE_PERF_GUARD("RenderGraphBuilder::LoadResourcesDesc");

		// Find list of resources that are in the config (they will form RIDs later on)
		std::vector<std::string_view> presentResources;
		for (const auto& res : desc.Resources)
		{
			ZE_CHECK_FAILED_CONFIG_LOAD((res.second.Flags & FrameResourceFlag::Texture3D) && (res.second.Flags & FrameResourceFlag::Cube),
				ErrorWrongResourceDimensionsFlags, "Cannot create cubemap texture as 3D texture for resource [" + res.first + "]!");

			ZE_CHECK_FAILED_CONFIG_LOAD((res.second.Flags & (FrameResourceFlag::ForceRTV | FrameResourceFlag::ForceUAV)) && (res.second.Flags & FrameResourceFlag::ForceDSV),
				ErrorIncorrectResourceUsage, "Cannot create depth stencil together with render target or unordered access view for same resource [" + res.first + "]!");

			ZE_CHECK_FAILED_CONFIG_LOAD((res.second.Flags & FrameResourceFlag::ForceRTV) && Utils::IsDepthStencilFormat(res.second.Format),
				ErrorIncorrectResourceFormat, "Cannot use depth stencil format with render target for resource [" + res.first + "]!");

			ZE_CHECK_FAILED_CONFIG_LOAD((res.second.Flags & FrameResourceFlag::Texture3D) && (res.second.Flags & FrameResourceFlag::ForceDSV),
				ErrorWrongResourceConfiguration, "Cannot create 3D texture as depth stencil in resource [" + res.first + "]!");

			ZE_CHECK_FAILED_CONFIG_LOAD((res.second.Flags & FrameResourceFlag::SimultaneousAccess) && (res.second.Flags & FrameResourceFlag::ForceDSV),
				ErrorWrongResourceConfiguration, "Simultaneous access cannot be used on depth stencil in resource [" + res.first + "]!");

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

	FrameBufferDesc RenderGraphBuilder::GetFrameBufferLayout() const noexcept
	{
		FrameBufferDesc desc = {};
		desc.Flags = resourceOptions;
		desc.PassLevelCount = dependencyLevelCount;

		// Begin | End level
		std::unordered_map<std::string_view, std::pair<U32, U32>> resourceLookup;
		resourceLookup.reserve(computedResources.size());
		desc.Resources.clear();
		desc.Resources.reserve(computedResources.size());
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
			U32 depStart = dependencyLevels.at(i);
			U32 depEnd = depStart + 1;
			const auto& computed = computedGraph.at(i);
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

		// Copy lifetimes to final structure
		desc.ResourceLifetimes.clear();
		desc.ResourceLifetimes.reserve(computedResources.size());
		for (const auto& resName : computedResources)
			desc.ResourceLifetimes.emplace_back(resourceLookup.at(resName));
		return desc;
	}

	void RenderGraphBuilder::GroupRenderPasses(Device& dev, Data::AssetsStreamer& assets, RenderGraph& graph, const RenderGraphDesc& desc) const
	{
		RendererPassBuildData buildData = { graph.execData.Bindings, assets, desc.SettingsRange, desc.DynamicDataRange, desc.Samplers };
		auto fillInPassData = [&](U32 node, RenderGraph::ParallelPassGroup::PassInfo& passInfo)
			{
				const PassDesc& nodeDesc = passDescs.at(node).at(computedGraph.at(node).NodeGroupIndex).GetDesc();

				passInfo.PassID = node;
				passInfo.Exec = nodeDesc.Execute;
				passInfo.Resources = GetNodeResources(node);
				passInfo.Data.Resources = passInfo.Resources.get();
				if (nodeDesc.Init)
					passInfo.Data.ExecData = nodeDesc.Init(dev, buildData, nodeDesc.InitializeFormats, nodeDesc.InitData);
			};

		if (asyncComputeEnabled)
		{
			graph.asyncListChain.Exec([&dev](CommandList& cmd) { cmd.Init(dev, QueueType::Compute); });

			// Create exec groups for every dependency level
			std::vector<std::vector<std::vector<U32>>> execGroups(dependencyLevelCount);
			std::vector<std::vector<std::vector<U32>>> execGroupsAsync(dependencyLevelCount);
			// 1 - GFX, 2 - Async, 3 - both
			std::vector<U8> execGroupWorkType(dependencyLevelCount);
			for (U32 i = 0, size = Utils::SafeCast<U32>(dependencyLevels.size()); i < size; ++i)
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
			graph.execGroupCount = 1;
			graph.passExecGroups = std::make_unique<std::array<RenderGraph::ExecutionGroup, 2>[]>(1);

			auto& execGroup = graph.passExecGroups[0].at(0);
			execGroup.PassGroupCount = dependencyLevelCount;
			execGroup.PassGroups = std::make_unique<RenderGraph::ParallelPassGroup[]>(dependencyLevelCount);

			// Get number of passes in each group and assign passes to those groups
			for (U32 i = 0, size = Utils::SafeCast<U32>(dependencyLevels.size()); i < size; ++i)
				++execGroup.PassGroups[dependencyLevels.at(i)].PassCount;
			auto passesIndices = std::make_unique<U32[]>(dependencyLevelCount);
			for (U32 level = 0; level < dependencyLevelCount; ++level)
			{
				passesIndices[level] = execGroup.PassGroups[level].PassCount;
				execGroup.PassGroups[level].Passes = std::make_unique<RenderGraph::ParallelPassGroup::PassInfo[]>(execGroup.PassGroups[level].PassCount);
			}
			// Fill in description of single pass in group
			for (U32 i = 0, size = Utils::SafeCast<U32>(dependencyLevels.size()); i < size; ++i)
			{
				U32 depLevel = dependencyLevels.at(i);
				auto& passGroup = execGroup.PassGroups[depLevel];
				U32 passIndex = passGroup.PassCount - passesIndices[depLevel]--;

				fillInPassData(i, passGroup.Passes[passIndex]);
			}
		}

		// Clear up loaded shaders
		for (auto& shader : buildData.ShaderCache)
			shader.second.Free(dev);
	}

	BuildResult RenderGraphBuilder::FillPassBarriers(RenderGraph& graph, GraphFinalizeFlags flags) noexcept
	{
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
		};

		// Map for quick resolution of RIDs
		std::unordered_map<std::string_view, RID> resourceLookup;
		resourceLookup.reserve(computedResources.size());
		for (RID i = 0; i < computedResources.size(); ++i)
			resourceLookup.emplace(computedResources.at(i), i);

		// Group all resources into lifetimes based on their usage
		std::vector<std::map<U32, ResourceState>> resourceLifetimes;
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

		// Compute what layout changes are needed between resource usages
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
						auto& endExecGroup = graph.passExecGroups[end.ExecGroupIndex].at(static_cast<U64>(end.AsyncQueue));

						if (begin.ExecGroupIndex == end.ExecGroupIndex)
						{
							placeTransition(endExecGroup.PassGroups[begin.PassGroupIndex + 1].StartBarriers,
								endExecGroup.PassGroups[end.PassGroupIndex].StartBarriers, barrier,
								begin.PassGroupIndex + 1 == end.PassGroupIndex);
						}
						else
						{
							auto& beginExecGroup = graph.passExecGroups[begin.ExecGroupIndex].at(static_cast<U64>(begin.AsyncQueue));

							// Different queues, have to execute transition on starting queue
							if (begin.AsyncQueue != end.AsyncQueue)
							{
								// Last group, move barrier to the end barriers section
								placeTransition(beginExecGroup.PassGroups[begin.PassGroupIndex + 1].StartBarriers,
									beginExecGroup.EndBarriers, barrier, begin.PassGroupIndex + 1 == beginExecGroup.PassGroupCount);
							}
							else
							{
								// Last pass group in exec group, prefer using end exec group for possibility of more barriers in one place
								if (begin.PassGroupIndex + 1 == beginExecGroup.PassGroupCount)
								{
									placeTransition(endExecGroup.PassGroups[0].StartBarriers,
										endExecGroup.PassGroups[end.PassGroupIndex].StartBarriers, barrier, false);
								}
								else if (flags & GraphFinalizeFlag::NoSplitBarriersUseBegin)
									beginExecGroup.PassGroups[begin.PassGroupIndex + 1].StartBarriers.emplace_back(barrier);
								else if (flags & GraphFinalizeFlag::NoSplitBarriersUseEnd)
									endExecGroup.PassGroups[end.PassGroupIndex].StartBarriers.emplace_back(barrier);
								else
								{
									barrier.Type = BarrierType::SplitBegin;
									if (flags & GraphFinalizeFlag::CrossExecGroupSplitBarriersUseEndGroup)
										beginExecGroup.PassGroups[begin.PassGroupIndex + 1].StartBarriers.emplace_back(barrier);
									else
										endExecGroup.PassGroups[0].StartBarriers.emplace_back(barrier);

									barrier.Type = BarrierType::SplitEnd;
									endExecGroup.PassGroups[end.PassGroupIndex].StartBarriers.emplace_back(barrier);
								}
							}
						}
					}
				}
			}
		}

		// Intial transitions for every resource + end wrapping transitions when resource is temporal
		for (RID rid = 0; rid < resourceLifetimes.size(); ++rid)
		{
			auto& lifetime = resourceLifetimes.at(rid);
			auto firstUsage = lifetime.begin();
			auto lastUsage = lifetime.end();

			if (resources.Get(std::string(computedResources.at(rid))).Flags & FrameResourceFlag::Temporal)
			{
				TextureLayout first = firstUsage->second.InputLayout;
				TextureLayout last = lastUsage->second.OutputLayout;
				if (first != last)
				{
					BarrierTransition barrier =
					{
						rid, lastUsage->second.OutputLayout, firstUsage->second.InputLayout,
						lastUsage->second.PossibleAccess, firstUsage->second.PossibleAccess,
						GetSyncFromAccess(lastUsage->second.PossibleAccess, lastUsage->second.WorkGfx, lastUsage->second.WorkCompute, lastUsage->second.WorkRayTracing),
						GetSyncFromAccess(firstUsage->second.PossibleAccess, firstUsage->second.WorkGfx, firstUsage->second.WorkCompute, firstUsage->second.WorkRayTracing),
						BarrierType::Immediate
					};
				}
			}
			else
			{
				BarrierTransition barrier =
				{
					rid, TextureLayout::Undefined, firstUsage->second.InputLayout,
					static_cast<ResourceAccesses>(ResourceAccess::None), firstUsage->second.PossibleAccess,
					static_cast<StageSyncs>(StageSync::None),
					GetSyncFromAccess(firstUsage->second.PossibleAccess, firstUsage->second.WorkGfx, firstUsage->second.WorkCompute, firstUsage->second.WorkRayTracing),
					BarrierType::Immediate
				};
			}
		}

		return BuildResult::Success;
	}

	BuildResult RenderGraphBuilder::LoadConfig(const RenderGraphDesc& desc) noexcept
	{
		// Clear previous config on start for sanity
		ClearConfig();

		// Can be run on multiple threads possibly
		BuildResult result = LoadGraphDesc(desc);
		if (result != BuildResult::Success)
		{
			ClearConfig();
			return result;
		}

		result = LoadResourcesDesc(desc);
		if (result != BuildResult::Success)
		{
			ClearConfig();
			return result;
		}
		return BuildResult::Success;
	}

	BuildResult RenderGraphBuilder::ComputeGraph(bool minimizeDistances) noexcept
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

		ClearComputedGraph();

		// Check for presence of nodes in current configuration, first by evaluation value
		std::vector<PresenceInfo> presentNodes(passDescs.size());
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

		// Create adjacency graph for current configuration from dependency lists
		std::vector<std::vector<U32>> graphList(passDescs.size());
		for (U32 i = 0; i < graphList.size(); ++i)
		{
			for (const auto& prevNode : renderGraphDepList.at(i).NodesDependecies.at(presentNodes.at(i).NodeGroupIndex).PreceedingNodes)
			{
				graphList.at(prevNode.NodeIndex).emplace_back(i);
			}
		}

		// Check present processor nodes if they have all required input from producers and if their output is consumed by some producers
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

		// Only passes with both input and outputs will be present in the computed graph
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

		// Fill real in/out resources and dependency levels
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

		// Minimize distances between nodes when possible
		if (minimizeDistances)
		{
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
		auto getFlagsFromLayout = [](TextureLayout layout) -> FrameResourceFlags
			{
				FrameResourceFlags flags = static_cast<FrameResourceFlags>(FrameResourceFlag::InternalResourceActive);
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
				if ((res.Flags & FrameResourceFlag::Texture3D) && (res.Flags & FrameResourceFlag::InternalUsageDepth))
				{
					ZE_FAIL("Cannot create 3D texture as depth stencil in resource [" + name + "]!");
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

		// Get final list of resources
		computedResources.emplace_back(BACKBUFFER_NAME);
		resources.Transform([&](const std::string& name, const FrameResourceDesc& res)
			{
				if ((res.Flags & FrameResourceFlag::InternalResourceActive) && name != BACKBUFFER_NAME)
					computedResources.emplace_back(name);
			});

		// TODO: need to have update path when FFX SDK requests new buffers as it happens not during graph recompilation but after
		//       initializing it's internal structures where buffers are "created" (added to list for next recompile)
		//       either run the compute step after initializing all the new render passes or make a path for resource only update

		return result;
	}

	BuildResult RenderGraphBuilder::FinalizeGraph(Graphics& gfx, Data::AssetsStreamer& assets, RenderGraph& graph, const RenderGraphDesc& desc, GraphFinalizeFlags flags)
	{
		// Not handled initialization yet:
		// RendererDynamicData DynamicData;
		// RendererGraphData GraphData;

		// In case that graph have not been yet computed
		if (!IsGraphComputed())
		{
			ZE_WARNING("Graph needs to be computed before finalizing it!");
			BuildResult result = ComputeGraph();
			if (result != BuildResult::Success)
				return result;
		}
		Device& dev = gfx.GetDevice();

		// Create GPU buffers
		graph.execData.DynamicBuffer = &gfx.GetDynamicBuffer();
		//graph.execData.Buffers.Init(dev, GetFrameBufferLayout());

		// Send to GPU new graph data
		graph.execData.CustomData = desc.PassCustomData;
		graph.execData.SettingsData = desc.SettingsData;

		Resource::CBufferData settingsData = {};
		settingsData.DataStatic = &graph.execData.SettingsData;
		settingsData.Bytes = sizeof(RendererSettingsData);
		graph.execData.SettingsBuffer.Init(dev, assets.GetDisk(), settingsData);
		assets.GetDisk().StartUploadGPU(true);

		// Initialize passes structure
		GroupRenderPasses(dev, assets, graph, desc);

		// Skip computation of barriers where not required
		if (Settings::GetGfxApi() != GfxApiType::DX11 && Settings::GetGfxApi() != GfxApiType::OpenGL)
			return FillPassBarriers(graph, flags);
		return BuildResult::Success;
	}

	void RenderGraphBuilder::ClearConfig() noexcept
	{
		resourceOptions = static_cast<FrameBufferFlags>(FrameBufferFlag::None);
		resources.Clear();
		passDescs.clear();
		renderGraphDepList.clear();
		topoplogyOrder.clear();

		ClearComputedGraph();
	}

	void RenderGraphBuilder::ClearComputedGraph() noexcept
	{
		computedGraph.clear();
		dependencyLevels.clear();
		computedResources.clear();
		asyncComputeEnabled = false;
		dependencyLevelCount = 0;
		resources.Transform([](FrameResourceDesc& desc) { desc.Flags &= ~FrameResourceFlag::InternalFlagsMask; });
	}
}