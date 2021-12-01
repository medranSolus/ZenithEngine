#include "GFX/Pipeline/RenderGraph.h"

namespace ZE::GFX::Pipeline
{
	void RenderGraph::BeforeSync(Device& dev, const PassSyncDesc& syncInfo)
	{
		// Insert waits into correnct GPU engines
		switch (syncInfo.EnterSync)
		{
		case SyncType::MainToAll:
			dev.WaitMainFromCopy(syncInfo.EnterFence2);
		case SyncType::MainToCompute:
		{
			dev.WaitMainFromCompute(syncInfo.EnterFence1);
			break;
		}
		case SyncType::MainToCopy:
		{
			dev.WaitMainFromCopy(syncInfo.EnterFence2);
			break;
		}
		case SyncType::ComputeToAll:
			dev.WaitComputeFromCopy(syncInfo.EnterFence2);
		case SyncType::ComputeToMain:
		{
			dev.WaitComputeFromMain(syncInfo.EnterFence1);
			break;
		}
		case SyncType::ComputeToCopy:
		{
			dev.WaitComputeFromCopy(syncInfo.EnterFence2);
			break;
		}
		case SyncType::CopyToAll:
			dev.WaitCopyFromCompute(syncInfo.EnterFence2);
		case SyncType::CopyToMain:
		{
			dev.WaitCopyFromMain(syncInfo.EnterFence1);
			break;
		}
		case SyncType::CopyToCompute:
		{
			dev.WaitCopyFromCompute(syncInfo.EnterFence2);
			break;
		}
		}
	}

	void RenderGraph::AfterSync(Device& dev, PassSyncDesc& syncInfo)
	{
		// Set fences for correct GPU engines
		U64 fence1 = 0;
		U64 fence2 = 0;
		switch (syncInfo.AllExitSyncs)
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

		// Update fence values for all passes that depends on current pass
		assert(syncInfo.DependentsCount <= 255);
		for (U8 i = 0; i < syncInfo.DependentsCount; ++i)
		{
			U64 nextFence = 0;
			ExitSync exitSyncInfo = syncInfo.ExitSyncs[i];
			switch (exitSyncInfo.Type)
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
			U64 currentFence = *exitSyncInfo.NextPassFence;
			// Atomicitly check if current stored fence value is highest
			// if no then store new value (or try if other thread also tries to store)
			while (currentFence < nextFence)
				if (exitSyncInfo.NextPassFence->compare_exchange_weak(currentFence, nextFence, std::memory_order::relaxed))
					break;
		}
	}

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

	void RenderGraph::ExecuteThread(Device& dev, PassDesc& pass)
	{
		//BeforeSync(dev, pass.Syncs);
		//pass.Execute(pass.Data);
		//AfterSync(dev, pass.Syncs);
	}

	void RenderGraph::Finalize(Device& dev, CommandList& mainList, std::vector<RenderNode>& nodes, FrameBufferDesc& frameBufferDesc, bool minimizeDistances)
	{
		// Create graph via adjacency list
		std::vector<std::vector<U64>> graphList(nodes.size());
		std::vector<std::vector<U64>> syncList(nodes.size());
		std::vector<std::vector<U64>> depList(nodes.size());
		U32 staticCount = 0;
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
			if (currentNode.IsStatic())
				++staticCount;
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

		// Minimize distances between nodes when possible
		if (minimizeDistances)
		{
			for (auto& list : graphList)
				list.clear();
			U64 i = 0;
			for (const auto& deps : depList)
			{
				U64 currentLevel = dependencyLevels.at(i++);
				for (U64 dep : deps)
					graphList.at(dep).emplace_back(currentLevel);
			}
			i = 0;
			for (auto& list : graphList)
			{
				if (list.size() > 0)
				{
					std::sort(list.begin(), list.end());
					dependencyLevels.at(i) = list.front() - 1;
				}
				++i;
			}
		}
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
						const U64 rid = it->GetOutputResources().at(k);
						if (rid >= frameBufferDesc.ResourceInfo.size())
							throw ZE_RGC_EXCEPT("Cannot find resource for input [" + input + "], RID out of range [" + std::to_string(rid) + "]!");

						node.AddInputResource(rid);
						Resource::State currentState = node.GetInputState(j);
						auto& lifetime = frameBufferDesc.ResourceLifetimes.at(rid);
						if (lifetime.contains(depLevel))
						{
							Resource::State presentState = lifetime.at(depLevel);
							if (presentState != currentState)
							{
								if (presentState == Resource::State::ShaderResourceNonPS && currentState == Resource::State::ShaderResourcePS
									|| currentState == Resource::State::ShaderResourceNonPS && presentState == Resource::State::ShaderResourcePS)
								{
									lifetime.at(depLevel) = Resource::State::ShaderResourceAll;
								}
								else if (presentState != Resource::State::ShaderResourceAll
									|| currentState != Resource::State::ShaderResourcePS && currentState != Resource::State::ShaderResourceNonPS)
								{
									throw ZE_RGC_EXCEPT("Resource [" + std::to_string(rid) + "] cannot be at same dependency level [" +
										std::to_string(depLevel) + "] in 2 disjunctive states!");
								}
							}
						}
						else
							lifetime[depLevel] = currentState;
						break;
					}
					else if (++k == it->GetOutputs().size())
						throw ZE_RGC_EXCEPT("Cannot find source for input [" + input + "]!");
				}
				++j;
			}
			// Check all output resources
			for (U64 j = 0; const U64 rid : node.GetOutputResources())
			{
				if (rid >= frameBufferDesc.ResourceInfo.size())
					throw ZE_RGC_EXCEPT("RID out of range [" + std::to_string(rid) + "] for output + [" + node.GetOutputs().at(j) + "]!");

				Resource::State currentState = node.GetOutputState(j);
				auto& lifetime = frameBufferDesc.ResourceLifetimes.at(rid);
				if (lifetime.contains(depLevel))
				{
					Resource::State presentState = lifetime.at(depLevel);
					if (presentState != currentState)
					{
						if (presentState == Resource::State::ShaderResourceNonPS && currentState == Resource::State::ShaderResourcePS
							|| currentState == Resource::State::ShaderResourceNonPS && presentState == Resource::State::ShaderResourcePS)
						{
							lifetime.at(depLevel) = Resource::State::ShaderResourceAll;
						}
						else if (presentState != Resource::State::ShaderResourceAll
							|| currentState != Resource::State::ShaderResourcePS && currentState != Resource::State::ShaderResourceNonPS)
						{
							throw ZE_RGC_EXCEPT("Resource [" + std::to_string(rid) + "] cannot be at same dependency level [" +
								std::to_string(depLevel) + "] in 2 disjunctive states!");
						}
					}
				}
				else
					lifetime[depLevel] = currentState;
				++j;
			}
			// Check temporary inner resources
			for (auto& innerBuffer : node.GetInnerBuffers())
			{
				node.AddInnerBufferResource(frameBufferDesc.AddResource(std::move(innerBuffer.Info)));
				frameBufferDesc.ResourceLifetimes.back()[depLevel] = innerBuffer.InitState;
			}
		}

		// Sort passes into dependency level groups of execution
		passes = new std::pair<PassDesc*, U64>[++levelCount];
		passesCleaners = new PassCleanCallback * [levelCount];
		U64 normalCount = nodes.size() - staticCount;
		passes[0].first = new PassDesc[normalCount];
		passesCleaners[0] = new PassCleanCallback[normalCount];
		staticPasses = new PassDescStatic[levelCount];
		staticPasses[0].Commands = new CommandList[staticCount];
		staticCount = 0;
		// Static | Level | Location
		std::vector<std::pair<bool, std::pair<U64, U64>>> passLocation(nodes.size(), { false, { 0, 0 } });
		for (U64 i = 0; i < levelCount; ++i)
		{
			if (i != 0)
			{
				U64 prevCount = passes[i - 1].second;
				passes[i].first = passes[i - 1].first + prevCount;
				passesCleaners[i] = passesCleaners[i - 1] + prevCount;
				staticPasses[i].Commands = staticPasses[i - 1].Commands + staticPasses[i - 1].CommandsCount;
			}
			passes[i].second = 0;
			for (U64 j = 0; j < nodes.size(); ++j)
			{
				if (dependencyLevels.at(j) == i)
				{
					auto& node = nodes.at(j);
					PassSyncDesc* passSync;
					if (node.IsStatic())
					{
						auto& pass = staticPasses[i];
						passSync = &pass.Syncs;
						pass.Commands[pass.CommandsCount] = node.GetStaticExecuteData();
						passLocation.at(j) = { true, { i, pass.CommandsCount++ } };
						if (pass.CommandsCount > staticCount)
							staticCount = pass.CommandsCount;
					}
					else
					{
						auto& pass = passes[i].first[passes[i].second];
						passSync = &pass.Syncs;
						pass.Execute = node.GetExecuteCallback();
						pass.Data.Buffers = node.GetNodeRIDs();
						pass.Data.OptData = node.GetExecuteData();
						passesCleaners[i][passes[i].second] = node.GetCleanCallback();
						assert((pass.Data.OptData != nullptr) == (node.GetCleanCallback() != nullptr)
							&& "Optional data and cleaning function must be both provided or neither of them!");
						passLocation.at(j).second = { i, passes[i].second++ };
						if (passes[i].second > workersCount)
							workersCount = passes[i].second;
					}

					// Go through dependent passes and append current pass to their syncs
					bool requiredFence1 = false, requiredFence2 = false;
					for (U64 dep : syncList.at(j))
					{
						PassSyncDesc* depPassSync;
						if (passLocation.at(dep).first)
							depPassSync = &staticPasses[passLocation.at(dep).second.first].Syncs;
						else
							depPassSync = &passes[passLocation.at(dep).second.first].first[passLocation.at(dep).second.second].Syncs;
						assert(depPassSync->DependentsCount <= 255);
						depPassSync->ExitSyncs = reinterpret_cast<ExitSync*>(realloc(depPassSync->ExitSyncs, sizeof(ExitSync) * ++depPassSync->DependentsCount));
						auto& exitSync = depPassSync->ExitSyncs[depPassSync->DependentsCount - 1];
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
								exitSync.NextPassFence = &passSync->EnterFence1;
								break;
							}
							case QueueType::Copy:
							{
								requiredFence2 = true;
								exitSync.Type = SyncType::CopyToMain;
								exitSync.NextPassFence = &passSync->EnterFence2;
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
								exitSync.NextPassFence = &passSync->EnterFence1;
								break;
							}
							case QueueType::Compute:
								throw ZE_RGC_EXCEPT("Trying to create sync point between same GPU engine type! Bug in culling redundant nodes!");
							case QueueType::Copy:
							{
								requiredFence2 = true;
								exitSync.Type = SyncType::CopyToCompute;
								exitSync.NextPassFence = &passSync->EnterFence2;
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
								exitSync.NextPassFence = &passSync->EnterFence1;
								break;
							}
							case QueueType::Compute:
							{
								requiredFence2 = true;
								exitSync.Type = SyncType::ComputeToCopy;
								exitSync.NextPassFence = &passSync->EnterFence2;
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
							passSync->EnterSync = SyncType::MainToAll;
						else if (requiredFence1)
							passSync->EnterSync = SyncType::MainToCompute;
						else if (requiredFence2)
							passSync->EnterSync = SyncType::MainToCopy;
						break;
					}
					case QueueType::Compute:
					{
						if (requiredFence1 && requiredFence2)
							passSync->EnterSync = SyncType::ComputeToAll;
						else if (requiredFence1)
							passSync->EnterSync = SyncType::ComputeToMain;
						else if (requiredFence2)
							passSync->EnterSync = SyncType::ComputeToCopy;
						break;
					}
					case QueueType::Copy:
					{
						if (requiredFence1 && requiredFence2)
							passSync->EnterSync = SyncType::CopyToAll;
						else if (requiredFence1)
							passSync->EnterSync = SyncType::CopyToMain;
						else if (requiredFence2)
							passSync->EnterSync = SyncType::CopyToCompute;
						break;
					}
					}
				}
			}
		}
		for (U64 i = 0; i < nodes.size(); ++i)
		{
			PassSyncDesc* passSync;
			if (nodes.at(i).IsStatic())
				passSync = &staticPasses[passLocation.at(i).first].Syncs;
			else
				passSync = &passes[0].first[i].Syncs;
			assert(passSync->DependentsCount <= 255);
			bool requiredFence1 = false, requiredFence2 = false;
			for (U8 i = 0; i < passSync->DependentsCount; ++i)
			{
				ExitSync syncInfo = passSync->ExitSyncs[i];
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
			switch (passSync->EnterSync)
			{
			case SyncType::MainToAll:
			case SyncType::MainToCompute:
			case SyncType::MainToCopy:
			{
				if (requiredFence1 && requiredFence2)
					passSync->AllExitSyncs = SyncType::MainToAll;
				else if (requiredFence1)
					passSync->AllExitSyncs = SyncType::MainToCompute;
				else if (requiredFence2)
					passSync->AllExitSyncs = SyncType::MainToCopy;
				break;
			}
			case SyncType::ComputeToAll:
			case SyncType::ComputeToMain:
			case SyncType::ComputeToCopy:
			{
				if (requiredFence1 && requiredFence2)
					passSync->AllExitSyncs = SyncType::ComputeToAll;
				else if (requiredFence1)
					passSync->AllExitSyncs = SyncType::ComputeToMain;
				else if (requiredFence2)
					passSync->AllExitSyncs = SyncType::ComputeToCopy;
				break;
			}
			case SyncType::CopyToAll:
			case SyncType::CopyToMain:
			case SyncType::CopyToCompute:
			{
				if (requiredFence1 && requiredFence2)
					passSync->AllExitSyncs = SyncType::CopyToAll;
				else if (requiredFence1)
					passSync->AllExitSyncs = SyncType::CopyToMain;
				else if (requiredFence2)
					passSync->AllExitSyncs = SyncType::CopyToCompute;
				break;
			}
			}
		}

		dev.SetCommandBufferSize(staticCount);
		workerThreads = new std::thread[--workersCount];

		frameBufferDesc.ComputeWorkflowTransitions(levelCount);
		frameBuffer.Init(dev, mainList, frameBufferDesc);

		// Compute static passes
		for (U64 i = 0; const auto & node : nodes)
		{
			const auto& location = passLocation.at(i);
			if (location.first)
			{
				PassDescStatic& level = staticPasses[location.second.first];
				PassData staticPassData;
				staticPassData.Buffers = node.GetNodeRIDs();
				// Optional data is not supported for static RenderPass
				staticPassData.OptData = nullptr;
				CommandList& cl = level.Commands[location.second.second];
				cl.Open(dev);
				node.GetExecuteCallback()(cl, staticPassData);
				cl.Close(dev);
				delete[] staticPassData.Buffers;
			}
			++i;
		}
	}

	RenderGraph::~RenderGraph()
	{
		if (workerThreads)
			delete[] workerThreads;
		if (passes)
		{
			for (U64 i = 0; i < levelCount; ++i)
			{
				auto& level = passes[i];
				for (U64 j = 0; j < level.second; ++j)
				{
					auto& pass = level.first[j];
					if (pass.Data.Buffers)
						delete[] pass.Data.Buffers;
					if (pass.Data.OptData)
						passesCleaners[i][j](pass.Data.OptData);
					if (pass.Syncs.ExitSyncs)
						free(pass.Syncs.ExitSyncs);
				}
				if (staticPasses[i].Syncs.ExitSyncs)
					free(staticPasses[i].Syncs.ExitSyncs);
			}
			if (passes[0].first)
				delete[] passes[0].first;
			delete[] passes;
			if (passesCleaners[0])
				delete[] passesCleaners[0];
			delete[] passesCleaners;
		}
		if (staticPasses)
		{
			if (staticPasses[0].Commands)
				delete[] staticPasses[0].Commands;
			delete[] staticPasses;
		}
	}

	void RenderGraph::Execute(Device& dev, CommandList& mainList)
	{
		frameBuffer.InitTransitions(dev, mainList);
		for (U64 i = 0; i < levelCount; ++i)
		{
			mainList.Open(dev);
			auto& staticLevel = staticPasses[i];
			if (staticLevel.CommandsCount)
			{
				BeforeSync(dev, staticLevel.Syncs);
				dev.Execute(staticLevel.Commands, staticLevel.CommandsCount);
				AfterSync(dev, staticLevel.Syncs);
			}
			auto& level = passes[i];
			if (level.second)
			{
				U64 workersDispatch = level.second - 1;
				assert(workersCount >= workersDispatch);
				for (U64 j = 0; j < workersDispatch; ++j)
					workerThreads[j] = { &RenderGraph::ExecuteThread, this, std::ref(dev), std::ref(level.first[j]) };
				ExecuteThread(dev, level.first[workersDispatch]);
				for (U64 j = 0; j < workersDispatch; ++j)
					workerThreads[j].join();
			}
			frameBuffer.ExitTransitions(i, mainList);
			mainList.Close(dev);
			dev.ExecuteMain(mainList);
		}
	}
}