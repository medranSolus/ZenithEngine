#include "GFX/Pipeline/RenderGraph.h"

namespace ZE::GFX::Pipeline
{
	void RenderGraph::BeforeSync(Device& dev, const PassSyncDesc& syncInfo)
	{
		// Insert waits into correnct GPU engines
		switch (syncInfo.EnterSync)
		{
		case SyncType::AllToMain:
			dev.WaitMainFromCopy(syncInfo.EnterFence2);
		case SyncType::ComputeToMain:
		{
			dev.WaitMainFromCompute(syncInfo.EnterFence1);
			break;
		}
		case SyncType::CopyToMain:
		{
			dev.WaitMainFromCopy(syncInfo.EnterFence2);
			break;
		}
		case SyncType::AllToCompute:
			dev.WaitComputeFromCopy(syncInfo.EnterFence2);
		case SyncType::MainToCompute:
		{
			dev.WaitComputeFromMain(syncInfo.EnterFence1);
			break;
		}
		case SyncType::CopyToCompute:
		{
			dev.WaitComputeFromCopy(syncInfo.EnterFence2);
			break;
		}
		case SyncType::AllToCopy:
			dev.WaitCopyFromCompute(syncInfo.EnterFence2);
		case SyncType::MainToCopy:
		{
			dev.WaitCopyFromMain(syncInfo.EnterFence1);
			break;
		}
		case SyncType::ComputeToCopy:
		{
			dev.WaitCopyFromCompute(syncInfo.EnterFence2);
			break;
		}
		default:
			ZE_ASSERT(false, "Incorret enum value for enter sync context!");
		case SyncType::None:
			break;
		}
	}

	void RenderGraph::AfterSync(Device& dev, PassSyncDesc& syncInfo)
	{
		// Set fences for correct GPU engines
		U64 nextFence = 0;
		switch (syncInfo.AllExitSyncs)
		{
		case SyncType::MainToAll:
		case SyncType::MainToCompute:
		case SyncType::MainToCopy:
		{
			nextFence = dev.SetMainFence();
			break;
		}
		case SyncType::ComputeToAll:
		case SyncType::ComputeToMain:
		case SyncType::ComputeToCopy:
		{
			nextFence = dev.SetComputeFence();
			break;
		}
		case SyncType::CopyToAll:
		case SyncType::CopyToMain:
		case SyncType::CopyToCompute:
		{
			nextFence = dev.SetCopyFence();
			break;
		}
		default:
			ZE_ASSERT(false, "Incorret enum value for exit sync context!");
		case SyncType::None:
		{
			ZE_ASSERT(syncInfo.DependentsCount == 0, "No syncing performed while there are dependent passes!");
			return;
		}
		}

		// Update fence values for all passes that depends on current pass
		for (U8 i = 0; i < syncInfo.DependentsCount; ++i)
		{
			ExitSync exitSyncInfo = syncInfo.ExitSyncs[i];
#ifdef _ZE_RENDER_GRAPH_SINGLE_THREAD
			(*exitSyncInfo.NextPassFence) = nextFence;
#else
			U64 currentFence = *exitSyncInfo.NextPassFence;
			// Atomicitly check if current stored fence value is highest
			// if no then store new value (or try if other thread also tries to store)
			while (currentFence < nextFence)
				if (exitSyncInfo.NextPassFence->compare_exchange_weak(currentFence, nextFence, std::memory_order::relaxed))
					break;
#endif
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

	void RenderGraph::AssignState(Resource::State& presentState, Resource::State& currentState, RID rid, U64 depLevel)
	{
		if (presentState != currentState)
		{
			if (Resource::IsReadOnlyState(presentState) && Resource::IsReadOnlyState(currentState))
			{
				presentState |= currentState;
			}
			else if (Resource::IsWriteEnabledState(presentState) || Resource::IsWriteEnabledState(currentState))
			{
				throw ZE_RGC_EXCEPT("Resource [" + std::to_string(rid) + "] cannot be at same dependency level [" +
					std::to_string(depLevel) + "] in 2 write states at once! Wrong graph definition.");
			}
			else
			{
				throw ZE_RGC_EXCEPT("Resource [" + std::to_string(rid) + "] cannot be at same dependency level [" +
					std::to_string(depLevel) + "] in 2 disjunctive states!");
			}
		}
	}

	void RenderGraph::ExecuteThread(Device& dev, CommandList& cl, PassDesc& pass)
	{
		pass.Execute(dev, cl, execData, pass.Data);
	}

	void RenderGraph::ExecuteThreadSync(Device& dev, CommandList& cl, PassDesc& pass)
	{
		BeforeSync(dev, pass.Syncs);
		ExecuteThread(dev, cl, pass);
		AfterSync(dev, pass.Syncs);
	}

	void RenderGraph::Finalize(Device& dev, CommandList& mainList, std::vector<RenderNode>& nodes,
		FrameBufferDesc& frameBufferDesc, RendererBuildData& buildData, bool minimizeDistances)
	{
		execData.DynamicBuffers.Exec([&dev](auto& x) { x.Init(dev); });

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
			for (U64 j = 0; const auto& input : node.GetInputs())
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
							AssignState(lifetime.at(depLevel), currentState, rid, depLevel);
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
					AssignState(lifetime.at(depLevel), currentState, rid, depLevel);
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
		passes = new std::pair<Ptr<PassDesc>, U64>[++levelCount];
		passesCleaners = new Ptr<PassCleanCallback>[levelCount];
		passes[0].first = new PassDesc[nodes.size()];
		passesCleaners[0] = new PassCleanCallback[nodes.size()];
		// Level | Location
		std::vector<std::pair<U64, U64>> passLocation(nodes.size(), { 0, 0 });
		for (U64 i = 0; i < levelCount; ++i)
		{
			if (i != 0)
			{
				U64 prevCount = passes[i - 1].second;
				passes[i].first = passes[i - 1].first + prevCount;
				passesCleaners[i] = passesCleaners[i - 1] + prevCount;
			}
			passes[i].second = 0;
			for (U64 j = 0; j < nodes.size(); ++j)
			{
				if (dependencyLevels.at(j) == i)
				{
					auto& node = nodes.at(j);
					ZE_ASSERT((node.GetExecuteData() != nullptr) == (node.GetCleanCallback() != nullptr),
						"Optional data and cleaning function must be both provided or neither of them!");

					auto& pass = passes[i].first[passes[i].second];
					pass.Execute = node.GetExecuteCallback();
					ZE_ASSERT(pass.Execute, "Empty execution callbak!");
					pass.Data.Buffers = node.GetNodeRIDs();
					pass.Data.OptData = node.GetExecuteData();
					passesCleaners[i][passes[i].second] = node.GetCleanCallback();
					passLocation.at(j) = { i, passes[i].second++ };
#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
					if (passes[i].second > workersCount)
						workersCount = passes[i].second;
#endif

					// Go through dependent passes and append current pass to their syncs
					bool requiredFence1 = false, requiredFence2 = false;
					for (U64 dep : syncList.at(j))
					{
						PassSyncDesc& depPassSync = passes[passLocation.at(dep).first].first[passLocation.at(dep).second].Syncs;
						ZE_ASSERT(depPassSync.DependentsCount < 255, "Need to change used data type!");

						depPassSync.ExitSyncs = reinterpret_cast<ExitSync*>(realloc(depPassSync.ExitSyncs, sizeof(ExitSync) * ++depPassSync.DependentsCount));
						auto& exitSync = depPassSync.ExitSyncs[depPassSync.DependentsCount - 1];
						switch (node.GetPassType())
						{
						case QueueType::Main:
						{
							switch (nodes.at(dep).GetPassType())
							{
							default:
								ZE_ASSERT(false, "Unhandled enum value!");
							case QueueType::Main:
								throw ZE_RGC_EXCEPT("Trying to create sync point between same GPU engine type! Bug in culling redundant nodes!");
							case QueueType::Compute:
							{
								requiredFence1 = true;
								exitSync.Type = SyncType::ComputeToMain;
								exitSync.NextPassFence = &pass.Syncs.EnterFence1;
								break;
							}
							case QueueType::Copy:
							{
								requiredFence2 = true;
								exitSync.Type = SyncType::CopyToMain;
								exitSync.NextPassFence = &pass.Syncs.EnterFence2;
								break;
							}
							}
							break;
						}
						case QueueType::Compute:
						{
							switch (nodes.at(dep).GetPassType())
							{
							default:
								ZE_ASSERT(false, "Unhandled enum value!");
							case QueueType::Main:
							{
								requiredFence1 = true;
								exitSync.Type = SyncType::MainToCompute;
								exitSync.NextPassFence = &pass.Syncs.EnterFence1;
								break;
							}
							case QueueType::Compute:
								throw ZE_RGC_EXCEPT("Trying to create sync point between same GPU engine type! Bug in culling redundant nodes!");
							case QueueType::Copy:
							{
								requiredFence2 = true;
								exitSync.Type = SyncType::CopyToCompute;
								exitSync.NextPassFence = &pass.Syncs.EnterFence2;
								break;
							}
							}
							break;
						}
						case QueueType::Copy:
						{
							switch (nodes.at(dep).GetPassType())
							{
							default:
								ZE_ASSERT(false, "Unhandled enum value!");
							case QueueType::Main:
							{
								requiredFence1 = true;
								exitSync.Type = SyncType::MainToCopy;
								exitSync.NextPassFence = &pass.Syncs.EnterFence1;
								break;
							}
							case QueueType::Compute:
							{
								requiredFence2 = true;
								exitSync.Type = SyncType::ComputeToCopy;
								exitSync.NextPassFence = &pass.Syncs.EnterFence2;
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
					default:
						ZE_ASSERT(false, "Unhandled enum value!");
					case QueueType::Main:
					{
						if (requiredFence1 && requiredFence2)
							pass.Syncs.EnterSync = SyncType::AllToMain;
						else if (requiredFence1)
							pass.Syncs.EnterSync = SyncType::ComputeToMain;
						else if (requiredFence2)
							pass.Syncs.EnterSync = SyncType::CopyToMain;
						break;
					}
					case QueueType::Compute:
					{
						if (requiredFence1 && requiredFence2)
							pass.Syncs.EnterSync = SyncType::AllToCompute;
						else if (requiredFence1)
							pass.Syncs.EnterSync = SyncType::MainToCompute;
						else if (requiredFence2)
							pass.Syncs.EnterSync = SyncType::CopyToCompute;
						break;
					}
					case QueueType::Copy:
					{
						if (requiredFence1 && requiredFence2)
							pass.Syncs.EnterSync = SyncType::AllToCopy;
						else if (requiredFence1)
							pass.Syncs.EnterSync = SyncType::MainToCopy;
						else if (requiredFence2)
							pass.Syncs.EnterSync = SyncType::ComputeToCopy;
						break;
					}
					}
				}
			}
		}
		// Merge info about required fences
		for (U64 i = 0; i < nodes.size(); ++i)
		{
			PassSyncDesc& passSync = passes[passLocation.at(i).first].first[passLocation.at(i).second].Syncs;
			ZE_ASSERT(passSync.DependentsCount <= 255, "Need to increase DependentsCount range!");

			bool requiredFence1 = false, requiredFence2 = false;
			for (U8 j = 0; j < passSync.DependentsCount; ++j)
			{
				switch (passSync.ExitSyncs[j].Type)
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
				default:
					break;
				}
			}

			switch (nodes.at(i).GetPassType())
			{
			default:
				ZE_ASSERT(false, "Unhandled enum value!");
			case QueueType::Main:
			{
				if (requiredFence1 && requiredFence2)
					passSync.AllExitSyncs = SyncType::MainToAll;
				else if (requiredFence1)
					passSync.AllExitSyncs = SyncType::MainToCompute;
				else if (requiredFence2)
					passSync.AllExitSyncs = SyncType::MainToCopy;
				break;
			}
			case QueueType::Compute:
			{
				if (requiredFence1 && requiredFence2)
					passSync.AllExitSyncs = SyncType::ComputeToAll;
				else if (requiredFence1)
					passSync.AllExitSyncs = SyncType::ComputeToMain;
				else if (requiredFence2)
					passSync.AllExitSyncs = SyncType::ComputeToCopy;
				break;
			}
			case QueueType::Copy:
			{
				if (requiredFence1 && requiredFence2)
					passSync.AllExitSyncs = SyncType::CopyToAll;
				else if (requiredFence1)
					passSync.AllExitSyncs = SyncType::CopyToMain;
				else if (requiredFence2)
					passSync.AllExitSyncs = SyncType::CopyToCompute;
				break;
			}
			}
		}

#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
		workerThreads = new std::pair<std::thread, ChainPool<CommandList>>[--workersCount];
		for (U64 i = 0; i < workersCount; ++i)
			workerThreads[i].second.Exec([&dev](auto& x) { x.Init(dev, CommandType::All); });
#endif

		frameBufferDesc.ComputeWorkflowTransitions(levelCount);
		execData.Buffers.Init(dev, mainList, frameBufferDesc);

		// Create shared gfx states
		if (buildData.PipelineStates.size())
		{
			execData.SharedStates = new Resource::PipelineStateGfx[buildData.PipelineStates.size()];
			for (U64 i = 0; const auto & state : buildData.PipelineStates)
				execData.SharedStates[i++].Init(dev, state.second.second.first, execData.Bindings.GetSchema(state.second.first));
		}
		passLocation.clear();
	}

	RenderGraph::RenderGraph(void* renderer, void* settingsData, void* dynamicData, U32 dynamicDataSize) noexcept
		: dynamicDataSize(dynamicDataSize)
	{
		execData.SettingsData = settingsData;
		execData.DynamicData = dynamicData;
		execData.Renderer = renderer;
	}

	RenderGraph::~RenderGraph()
	{
#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
		if (workerThreads)
			workerThreads.DeleteArray();
#endif
		if (passes)
		{
			for (U64 i = 0; i < levelCount; ++i)
			{
				auto& level = passes[i];
				for (U64 j = 0; j < level.second; ++j)
				{
					auto& pass = level.first[j];
					if (pass.Data.Buffers)
						pass.Data.Buffers.DeleteArray();
					if (pass.Data.OptData)
						passesCleaners[i][j](pass.Data.OptData);
					if (pass.Syncs.ExitSyncs)
						pass.Syncs.ExitSyncs.Free();
				}
			}
			if (passes[0].first)
				passes[0].first.DeleteArray();
			passes.DeleteArray();
			if (passesCleaners[0])
				passesCleaners[0].DeleteArray();
			passesCleaners.DeleteArray();
		}
		if (execData.SharedStates)
			execData.SharedStates.DeleteArray();
	}

	void RenderGraph::Execute(Graphics& gfx)
	{
		ZE_PERF_START("Rendering sync");
		Device& dev = gfx.GetDevice();
		CommandList& mainList = gfx.GetMainList();

		switch (Settings::GetGfxApi())
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case GfxApiType::DX11:
		case GfxApiType::OpenGL:
		{
			ZE_PERF_STOP();
			ZE_PERF_START("Rendering");
			execData.DynamicBuffers.Get().StartFrame(dev);
			execData.DynamicBuffers.Get().Alloc(dev, execData.DynamicData, dynamicDataSize);
			execData.Buffers.SwapBackbuffer(dev, gfx.GetSwapChain());

			for (U64 i = 0; i < levelCount; ++i)
			{
				ZE_DRAW_TAG_BEGIN_MAIN(dev, (L"Level " + std::to_wstring(i + 1)).c_str(), PixelVal::White);
				auto& level = passes[i];
				if (level.second)
				{
#ifdef _ZE_RENDER_GRAPH_SINGLE_THREAD
					for (U64 j = 0; j < level.second; ++j)
						ExecuteThread(dev, mainList, level.first[j]);
#else
					U64 workersDispatch = level.second - 1;
					ZE_ASSERT(workersCount >= workersDispatch, "Insufficient number of workers!");
					for (U64 j = 0; j < workersDispatch; ++j)
						workerThreads[j].first = { &RenderGraph::ExecuteThread, this, std::ref(dev), std::ref(workerThreads[j].second.Get()), std::ref(level.first[j]) };
					ExecuteThread(dev, mainList, level.first[workersDispatch]);
					for (U64 j = 0; j < workersDispatch; ++j)
						workerThreads[j].first.join();
#endif
				}
				execData.Buffers.ExitTransitions(dev, mainList, i);
				ZE_DRAW_TAG_END_MAIN(dev);
			}
			break;
		}
		case GfxApiType::DX12:
		case GfxApiType::Vulkan:
		{
			gfx.WaitForFrame();
			ZE_PERF_STOP();
			ZE_PERF_START("Rendering");
			execData.DynamicBuffers.Get().StartFrame(dev);
			execData.DynamicBuffers.Get().Alloc(dev, execData.DynamicData, dynamicDataSize);
			execData.Buffers.SwapBackbuffer(dev, gfx.GetSwapChain());

			mainList.Reset(dev);
#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
			for (U64 i = 0; i < workersCount; ++i)
				workerThreads[i].second.Get().Reset(dev);
#endif
			execData.Buffers.InitTransitions(dev, mainList);
			for (U64 i = 0; i < levelCount; ++i)
			{
				ZE_DRAW_TAG_BEGIN_MAIN(dev, (L"Level " + std::to_wstring(i + 1)).c_str(), PixelVal::White);
				auto& level = passes[i];
				if (level.second)
				{
#ifdef _ZE_RENDER_GRAPH_SINGLE_THREAD
					for (U64 j = 0; j < level.second; ++j)
						ExecuteThreadSync(dev, mainList, level.first[j]);
#else
					U64 workersDispatch = level.second - 1;
					ZE_ASSERT(workersCount >= workersDispatch, "Insufficient number of workers!");
					for (U64 j = 0; j < workersDispatch; ++j)
						workerThreads[j].first = { &RenderGraph::ExecuteThreadSync, this, std::ref(dev), std::ref(workerThreads[j].second.Get()), std::ref(level.first[j]) };
					ExecuteThreadSync(dev, mainList, level.first[workersDispatch]);
					for (U64 j = 0; j < workersDispatch; ++j)
						workerThreads[j].first.join();
#endif
				}
				execData.Buffers.ExitTransitions(dev, mainList, i);
				ZE_DRAW_TAG_END_MAIN(dev);
			}
			break;
		}
		}
	}
}