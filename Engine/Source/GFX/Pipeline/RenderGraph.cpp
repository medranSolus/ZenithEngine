#include "GFX/Pipeline/RenderGraph.h"
#include "Exception/RenderGraphCompileException.h"
#include "Utils.h"

namespace ZE::GFX::Pipeline
{
	void RenderGraph::SortNodes(U64 currentNode, U64& orderedCount, const std::vector<std::vector<U64>>& graphList,
		std::vector<U64>& nodes, std::vector<std::bitset<2>>& visited)
	{
		if (visited.at(currentNode)[0])
		{
			if (visited.at(currentNode)[1])
				throw ZE_RGC_EXCEPT("Found circular dependency in node [" + std::to_string(currentNode) +
					"]! Possible multiple outputs to same buffer.");
			return;
		}
		visited[currentNode][0] = true;
		if (graphList.at(currentNode).size() == 0)
		{
			nodes.at(--orderedCount) = currentNode;
			return;
		}
		visited.at(currentNode)[1] = true;
		for (U64 adjNode : graphList.at(currentNode))
			SortNodes(adjNode, orderedCount, graphList, nodes, visited);
		nodes.at(--orderedCount) = currentNode;
		visited.at(currentNode)[1] = false;
	}

	void RenderGraph::CullIndirectDependecies(U64 currentNode, U64 checkNode, U64 minDepLevel, std::vector<std::vector<U64>>& syncList,
		const std::vector<std::vector<U64>>& depList, const std::vector<U64>& dependencyLevels) noexcept
	{
		auto& checkSyncList = syncList.at(checkNode);
		auto& currentDepList = depList.at(currentNode);
		for (U64 i = 0; i < currentDepList.size(); ++i)
		{
			auto& deps = depList.at(currentDepList.at(i));
			for (U64 j = 0; j < checkSyncList.size();)
			{
				if (std::find(deps.begin(), deps.end(), checkSyncList.at(j)) != deps.end())
				{
					if (checkSyncList.erase(checkSyncList.begin() + j) == checkSyncList.end())
						return;
				}
				else
					++j;
			}
		}
		for (U64 dep : currentDepList)
		{
			if (dependencyLevels.at(dep) > minDepLevel)
			{
				CullIndirectDependecies(dep, checkNode, minDepLevel, syncList, depList, dependencyLevels);
				if (syncList.size() == 0)
					return;
			}
		}
	}

	void RenderGraph::Finalize(std::vector<RenderNode>& nodes, Resource::FrameBufferDesc& frameBufferDesc)
	{
		// Create graph via adjacency list
		std::vector<std::vector<U64>> graphList(nodes.size());
		std::vector<std::vector<U64>> syncList(nodes.size());
		std::vector<std::vector<U64>> depList(nodes.size());
		for (U64 i = 0; i < nodes.size(); ++i)
		{
			auto& currentNode = nodes.at(i);
			if (std::find_if(nodes.begin(), nodes.begin() + i, [&currentNode](const RenderNode& n) { return n.GetName() == currentNode.GetName(); }) != nodes.end()
				&& std::find_if(nodes.begin() + i + 1, nodes.end(), [&currentNode](const RenderNode& n) { return n.GetName() == currentNode.GetName(); }) != nodes.end())
				throw ZE_RGC_EXCEPT("Render graph already contains node of name [" + currentNode.GetName() + "]!");
			for (const auto& out : currentNode.GetOutputs())
			{
				for (U64 j = 0; j < nodes.size(); ++j)
				{
					if (nodes.at(j).ContainsInput(out))
					{
						if (j == i)
							throw ZE_RGC_EXCEPT("Output of pass [" + nodes.at(j).GetName() + "] specified as it's input!");
						graphList.at(i).emplace_back(j);
						if (std::find(depList.at(j).begin(), depList.at(j).end(), i) == depList.at(j).end())
						{
							depList.at(j).emplace_back(i);
							if (nodes.at(j).GetPassType() != currentNode.GetPassType())
								syncList.at(j).emplace_back(i);
						}
					}
				}
			}
		}

		// Sort nodes in topological order
		std::vector<U64> ordered(nodes.size());
		U64 orderedCount = nodes.size();
		// Visited | on current stack of graph traversal
		std::vector<std::bitset<2>> visited(nodes.size(), 0);
		for (U64 i = 0; i < nodes.size(); ++i)
			if (!visited.at(i)[0])
				SortNodes(i, orderedCount, graphList, ordered, visited);
		visited.clear();

		// Compute longest path for each node as it's dependency level
		std::vector<U64> dependencyLevels(nodes.size(), 0);
		for (U64 node : ordered)
		{
			U64 nodeLevel = dependencyLevels.at(node);
			for (U64 adjNode : graphList.at(node))
			{
				if (dependencyLevels.at(adjNode) <= nodeLevel)
					dependencyLevels.at(adjNode) = nodeLevel + 1;
				if (dependencyLevels.at(adjNode) > levelCount)
					levelCount = dependencyLevels.at(adjNode);
			}
		}
		ordered.clear();
		graphList.clear();

		// Cull redundant syncs with other nodes on same GPU engines
		for (auto& syncs : syncList)
		{
			std::vector<bool> keep(syncs.size(), true);
			for (U64 i = 0; i < syncs.size(); ++i)
			{
				auto& checkNode = nodes.at(syncs.at(i));
				U64 checkLevel = dependencyLevels.at(syncs.at(i));
				for (U64 j = 0; j < syncs.size(); ++j)
				{
					U64 node = syncs.at(j);
					if (checkNode.GetPassType() == nodes.at(node).GetPassType())
					{
						node = dependencyLevels.at(node);
						if (node < checkLevel)
							keep.at(j) = false;
						else if (node > checkLevel)
							keep.at(i) = false;
					}
				}
			}
			std::vector<U64> remainingNodes;
			for (U64 i = 0; i < syncs.size(); ++i)
				if (keep.at(i))
					remainingNodes.emplace_back(syncs.at(i));
			syncs = std::move(remainingNodes);
		}

		// Cull redundant syncs, checking indirect sync points
		for (U64 i = 0; i < nodes.size(); ++i)
		{
			if (syncList.at(i).size() != 0)
			{
				U64 minDepLevel = dependencyLevels.at(i) - 1;
				for (U64 j = 0; j < syncList.at(i).size(); ++j)
				{
					U64 depLevel = dependencyLevels.at(syncList.at(i).at(j));
					if (minDepLevel > depLevel)
						minDepLevel = depLevel;
				}
				CullIndirectDependecies(i, i, minDepLevel, syncList, depList, dependencyLevels);
			}
		}
		depList.clear();

		// Sort passes into dependency level groups of execution
		passes = new std::pair<PassDesc*, U64>[++levelCount];
		passes[0].first = new PassDesc[nodes.size()];
		passes[0].first[0].Data = new PassData[nodes.size()];
		for (U64 i = 1; i < nodes.size(); ++i)
			passes[0].first[i].Data = passes[0].first[i - 1].Data + 1;
		std::vector<U64> passLocation(nodes.size(), 0);
		for (U64 i = 0; i < levelCount; ++i)
		{
			if (i != 0)
				passes[i].first = passes[i - 1].first + passes[i - 1].second;
			passes[i].second = 0;
			for (U64 j = 0; j < nodes.size(); ++j)
			{
				if (dependencyLevels.at(j) == i)
				{
					passLocation.at(j) = passes[i].second;
					auto& pass = passes[i].first[passes[i].second++];
					auto& node = nodes.at(j);
					bool requiredFence1 = false, requiredFence2 = false;
					for (U64 dep : syncList.at(j))
					{
						auto& depPass = passes[dependencyLevels.at(dep)].first[passLocation.at(dep)];
						assert(depPass.DependentsCount < 255);
						depPass.ExitSyncs = reinterpret_cast<ExitSync*>(realloc(depPass.ExitSyncs, sizeof(ExitSync) * ++depPass.DependentsCount));
						auto& exitSync = depPass.ExitSyncs[depPass.DependentsCount - 1];
						switch (node.GetPassType())
						{
						case QueueType::Main:
						{
							switch (nodes.at(dep).GetPassType())
							{
							case QueueType::Main:
								throw ZE_RGC_EXCEPT("Trying to create sync point between same GPU engine type! Bug in culling redundant nodes!");
							case QueueType::Compute:
							{
								requiredFence1 = true;
								exitSync.Type = SyncType::ComputeToMain;
								exitSync.NextPassFence = &pass.EnterFence1;
								break;
							}
							case QueueType::Copy:
							{
								requiredFence2 = true;
								exitSync.Type = SyncType::CopyToMain;
								exitSync.NextPassFence = &pass.EnterFence2;
								break;
							}
							}
							break;
						}
						case QueueType::Compute:
						{
							switch (nodes.at(dep).GetPassType())
							{
							case QueueType::Main:
							{
								requiredFence1 = true;
								exitSync.Type = SyncType::MainToCompute;
								exitSync.NextPassFence = &pass.EnterFence1;
								break;
							}
							case QueueType::Compute:
								throw ZE_RGC_EXCEPT("Trying to create sync point between same GPU engine type! Bug in culling redundant nodes!");
							case QueueType::Copy:
							{
								requiredFence2 = true;
								exitSync.Type = SyncType::CopyToCompute;
								exitSync.NextPassFence = &pass.EnterFence2;
								break;
							}
							}
							break;
						}
						case QueueType::Copy:
						{
							switch (nodes.at(dep).GetPassType())
							{
							case QueueType::Main:
							{
								requiredFence1 = true;
								exitSync.Type = SyncType::MainToCopy;
								exitSync.NextPassFence = &pass.EnterFence1;
								break;
							}
							case QueueType::Compute:
							{
								requiredFence2 = true;
								exitSync.Type = SyncType::ComputeToCopy;
								exitSync.NextPassFence = &pass.EnterFence2;
								break;
							}
							case QueueType::Copy:
								throw ZE_RGC_EXCEPT("Trying to create sync point between same GPU engine type! Bug in culling redundant nodes!");
							}
							break;
						}
						}
					}
					switch (node.GetPassType())
					{
					case QueueType::Main:
					{
						if (requiredFence1 && requiredFence2)
							pass.EnterSync = SyncType::MainToAll;
						else if (requiredFence1)
							pass.EnterSync = SyncType::MainToCompute;
						else if (requiredFence2)
							pass.EnterSync = SyncType::MainToCopy;
						break;
					}
					case QueueType::Compute:
					{
						if (requiredFence1 && requiredFence2)
							pass.EnterSync = SyncType::ComputeToAll;
						else if (requiredFence1)
							pass.EnterSync = SyncType::ComputeToMain;
						else if (requiredFence2)
							pass.EnterSync = SyncType::ComputeToCopy;
						break;
					}
					case QueueType::Copy:
					{
						if (requiredFence1 && requiredFence2)
							pass.EnterSync = SyncType::CopyToAll;
						else if (requiredFence1)
							pass.EnterSync = SyncType::CopyToMain;
						else if (requiredFence2)
							pass.EnterSync = SyncType::CopyToCompute;
						break;
					}
					}

					pass.Execute = node.GetExecuteCallback();
					pass.Data->OptData = node.GetExecuteData();
				}
			}
		}
		for (U64 i = 0; i < nodes.size(); ++i)
		{
			auto& pass = passes[0].first[i];
			assert(pass.DependentsCount <= 255);
			bool requiredFence1 = false, requiredFence2 = false;
			for (U8 i = 0; i < pass.DependentsCount; ++i)
			{
				ExitSync syncInfo = pass.ExitSyncs[i];
				switch (syncInfo.Type)
				{
				case SyncType::MainToCompute:
				case SyncType::ComputeToMain:
				case SyncType::CopyToMain:
				{
					requiredFence1 = true;
					break;
				}
				case SyncType::MainToCopy:
				case SyncType::ComputeToCopy:
				case SyncType::CopyToCompute:
				{
					requiredFence2 = true;
					break;
				}
				}
			}
			switch (pass.EnterSync)
			{
			case SyncType::MainToAll:
			case SyncType::MainToCompute:
			case SyncType::MainToCopy:
			{
				if (requiredFence1 && requiredFence2)
					pass.AllExitSyncs = SyncType::MainToAll;
				else if (requiredFence1)
					pass.AllExitSyncs = SyncType::MainToCompute;
				else if (requiredFence2)
					pass.AllExitSyncs = SyncType::MainToCopy;
				break;
			}
			case SyncType::ComputeToAll:
			case SyncType::ComputeToMain:
			case SyncType::ComputeToCopy:
			{
				if (requiredFence1 && requiredFence2)
					pass.AllExitSyncs = SyncType::ComputeToAll;
				else if (requiredFence1)
					pass.AllExitSyncs = SyncType::ComputeToMain;
				else if (requiredFence2)
					pass.AllExitSyncs = SyncType::ComputeToCopy;
				break;
			}
			case SyncType::CopyToAll:
			case SyncType::CopyToMain:
			case SyncType::CopyToCompute:
			{
				if (requiredFence1 && requiredFence2)
					pass.AllExitSyncs = SyncType::CopyToAll;
				else if (requiredFence1)
					pass.AllExitSyncs = SyncType::CopyToMain;
				else if (requiredFence2)
					pass.AllExitSyncs = SyncType::CopyToCompute;
				break;
			}
			}
		}

		// Compute resource lifetimes based on dependency levels
		for (U64 i = 0; i < nodes.size(); ++i)
		{
			U64 depLevel = dependencyLevels.at(i);
			auto& node = nodes.at(i);
			// Check all inputs by resources connected to them from dependent nodes
			for (U64 j = 0; const auto & input : node.GetInputs())
			{
				auto splitInput = Utils::SplitString(input, ".");
				auto it = std::find_if(nodes.begin(), nodes.end(), [&splitInput](const RenderNode& n)
					{
						return n.GetName() == splitInput.front();
					});
				if (it == nodes.end())
					throw ZE_RGC_EXCEPT("Cannot find source node [" + splitInput.front() + "] for pass [" + node.GetName() + "]!");
				for (U64 k = 0; k < it->GetOutputs().size();)
				{
					if (it->GetOutputs().at(k) == input)
					{
						auto& resName = it->GetOutputResources().at(k);
						for (U64 l = 0; l < frameBufferDesc.ResourceNames.size(); ++l)
						{
							if (frameBufferDesc.ResourceNames.at(l) == resName)
							{
								Resource::State currentState = node.GetInputeState(j);
								auto& states = frameBufferDesc.ResourceLifetimes.at(l).States[depLevel];
								if (std::find_if(states.begin(), states.end(), [&currentState, &node](const auto& s)
									{ return s.first == currentState && s.second == node.GetPassType(); }) == states.end())
									states.emplace_back(currentState, node.GetPassType());
								goto finish_finding_input_dependency_resource;
							}
						}
						throw ZE_RGC_EXCEPT("Cannot find resource for input [" + input + "]!");
					}
					else if (++k == it->GetOutputs().size())
						throw ZE_RGC_EXCEPT("Cannot find source for input [" + input + "]!");
				}
			finish_finding_input_dependency_resource:
				++j;
			}
			// Check all output resources
			for (U64 j = 0; const auto & outRes : node.GetOutputResources())
			{
				if (outRes == BACKBUFFER_NAME)
				{
					// TODO something with swapchain...
				}
				else
				{
					for (U64 k = 0; k < frameBufferDesc.ResourceNames.size();)
					{
						if (frameBufferDesc.ResourceNames.at(k) == outRes)
						{
							auto& lifetime = frameBufferDesc.ResourceLifetimes.at(k);
							if (lifetime.StartDepLevel > depLevel)
								lifetime.StartDepLevel = depLevel;
							Resource::State currentState = node.GetOutputState(j);
							auto& states = lifetime.States[depLevel];
							if (std::find_if(states.begin(), states.end(), [&currentState, &node](const auto& s)
								{ return s.first == currentState && s.second == node.GetPassType(); }) == states.end())
								states.emplace_back(currentState, node.GetPassType());
							break;
						}
						else if (++k == frameBufferDesc.ResourceNames.size())
							throw ZE_RGC_EXCEPT("Cannot find resource [" + outRes + "] for output + [" + node.GetOutputs().at(j) + "]!");
					}
				}
				++j;
			}
			// Check temporary inner resources
			for (auto& innerBuffer : node.GetInnerBuffers())
			{
				std::string resName = innerBuffer.Name;
				frameBufferDesc.AddResource(std::move(resName), std::move(innerBuffer.Info), depLevel);
				frameBufferDesc.ResourceLifetimes.back().States[depLevel].emplace_back(innerBuffer.InitState, node.GetPassType());
			}
		}
	}

	RenderGraph::~RenderGraph()
	{
		for (U64 i = 0; i < levelCount; ++i)
		{
			auto& level = passes[i];
			for (U64 j = 0; j < level.second; ++j)
			{
				auto& pass = level.first[j];
				if (pass.Data->OptData)
					delete pass.Data->OptData;
				if (pass.ExitSyncs)
					free(pass.ExitSyncs);
			}
		}
		if (passes)
		{
			if (passes[0].first)
			{
				if (passes[0].first[0].Data)
					delete[] passes[0].first[0].Data;
				delete[] passes[0].first;
			}
			delete[] passes;
		}
	}

	void RenderGraph::Execute(Device& dev)
	{
		for (U64 i = 0; i < levelCount; ++i)
		{
			auto& level = passes[i];
			for (U64 j = 0; j < level.second; ++j)
			{
				auto& pass = level.first[j];
				switch (pass.EnterSync)
				{
				case SyncType::MainToAll:
					dev.WaitMainFromCopy(pass.EnterFence2);
				case SyncType::MainToCompute:
				{
					dev.WaitMainFromCompute(pass.EnterFence1);
					break;
				}
				case SyncType::MainToCopy:
				{
					dev.WaitMainFromCopy(pass.EnterFence2);
					break;
				}
				case SyncType::ComputeToAll:
					dev.WaitComputeFromCopy(pass.EnterFence2);
				case SyncType::ComputeToMain:
				{
					dev.WaitComputeFromMain(pass.EnterFence1);
					break;
				}
				case SyncType::ComputeToCopy:
				{
					dev.WaitComputeFromCopy(pass.EnterFence2);
					break;
				}
				case SyncType::CopyToAll:
					dev.WaitCopyFromCompute(pass.EnterFence2);
				case SyncType::CopyToMain:
				{
					dev.WaitCopyFromMain(pass.EnterFence1);
					break;
				}
				case SyncType::CopyToCompute:
				{
					dev.WaitCopyFromCompute(pass.EnterFence2);
					break;
				}
				}
				pass.Execute(pass.Data);
				U64 fence1 = 0;
				U64 fence2 = 0;
				switch (pass.AllExitSyncs)
				{
				case SyncType::MainToAll:
					fence2 = dev.SetCopyFenceFromMain();
				case SyncType::MainToCompute:
				{
					fence1 = dev.SetComputeFenceFromMain();
					break;
				}
				case SyncType::MainToCopy:
				{
					fence2 = dev.SetCopyFenceFromMain();
					break;
				}
				case SyncType::ComputeToAll:
					fence2 = dev.SetCopyFenceFromCompute();
				case SyncType::ComputeToMain:
				{
					fence1 = dev.SetMainFenceFromCompute();
					break;
				}
				case SyncType::ComputeToCopy:
				{
					fence2 = dev.SetCopyFenceFromCompute();
					break;
				}
				case SyncType::CopyToAll:
					fence2 = dev.SetComputeFenceFromCopy();
				case SyncType::CopyToMain:
				{
					fence1 = dev.SetMainFenceFromCopy();
					break;
				}
				case SyncType::CopyToCompute:
				{
					fence2 = dev.SetComputeFenceFromCopy();
					break;
				}
				}
				assert(pass.DependentsCount <= 255);
				for (U8 i = 0; i < pass.DependentsCount; ++i)
				{
					U64 nextFence = 0;
					ExitSync syncInfo = pass.ExitSyncs[i];
					switch (syncInfo.Type)
					{
					case SyncType::MainToCompute:
					{
						nextFence = fence1;
						break;
					}
					case SyncType::MainToCopy:
					{
						nextFence = fence2;
						break;
					}
					case SyncType::ComputeToMain:
					{
						nextFence = fence1;
						break;
					}
					case SyncType::ComputeToCopy:
					{
						nextFence = fence2;
						break;
					}
					case SyncType::CopyToMain:
					{
						nextFence = fence1;
						break;
					}
					case SyncType::CopyToCompute:
					{
						nextFence = fence2;
						break;
					}
					}
					U64 currentFence = *syncInfo.NextPassFence;
					// Atomicitly check if current stored fence value is highest
					// if no then store new value (or try if other thread also tries to store)
					while (currentFence < nextFence)
						if (syncInfo.NextPassFence->compare_exchange_weak(currentFence, nextFence, std::memory_order::relaxed))
							break;
				}
			}
		}
	}
}