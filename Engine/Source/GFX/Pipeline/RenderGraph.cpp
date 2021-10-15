#include "GFX/Pipeline/RenderGraph.h"
#include "Exception/RenderGraphCompileException.h"

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

	void RenderGraph::Finalize(const std::vector<RenderNode>& nodes)
	{
		// Create graph via adjacency list
		std::vector<std::vector<U64>> graphList(nodes.size());
		std::vector<std::vector<U64>> syncList(nodes.size());
		std::vector<std::vector<U64>> depList(nodes.size());
		for (U64 i = 0; i < nodes.size(); ++i)
		{
			for (const auto& out : nodes.at(i).GetOutputs())
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
							if (nodes.at(j).GetPassType() != nodes.at(i).GetPassType())
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
		for (U64 i = 0; i < levelCount; ++i)
		{
			if (i != 0)
				passes[i].first = passes[i - 1].first + passes[i - 1].second;
			passes[i].second = 0;
			for (U64 j = 0; j < nodes.size(); ++j)
			{
				if (dependencyLevels.at(j) == i)
				{
					auto& pass = passes[i].first[passes[i].second++];
					auto& node = nodes.at(j);
					bool requiredFence1 = false, requiredFence2 = false;
					for (U64 dep : syncList.at(j))
					{
						auto& depPass = passes[0].first[dep];
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
					pass.Data = node.GetExecuteData();
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
			switch (nodes.at(i).GetPassType())
			{
			case QueueType::Main:
			{
				if (requiredFence1 && requiredFence2)
					pass.AllExitSyncs = SyncType::MainToAll;
				else if (requiredFence1)
					pass.AllExitSyncs = SyncType::MainToCompute;
				else if (requiredFence2)
					pass.AllExitSyncs = SyncType::MainToCopy;
				break;
			}
			case QueueType::Compute:
			{
				if (requiredFence1 && requiredFence2)
					pass.AllExitSyncs = SyncType::ComputeToAll;
				else if (requiredFence1)
					pass.AllExitSyncs = SyncType::ComputeToMain;
				else if (requiredFence2)
					pass.AllExitSyncs = SyncType::ComputeToCopy;
				break;
			}
			case QueueType::Copy:
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
	}

	RenderGraph::RenderGraph()
	{
	}

	RenderGraph::~RenderGraph()
	{
		for (U64 i = 0; i < levelCount; ++i)
		{
			auto& level = passes[i];
			for (U64 j = 0; j < level.second; ++j)
			{
				auto& pass = level.first[j];
				if (pass.Data)
					delete pass.Data;
				if (pass.ExitSyncs)
					free(pass.ExitSyncs);
			}
		}
		delete[] passes[0].first;
		delete[] passes;
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
				pass.Enter();
				pass.Execute(pass.Data);
				pass.Exit();
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