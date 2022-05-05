#include "GFX/Pipeline/RenderGraph.h"

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

	void RenderGraph::Finalize(Graphics& gfx, std::vector<RenderNode>& nodes,
		FrameBufferDesc& frameBufferDesc, RendererBuildData& buildData, bool minimizeDistances)
	{
		execData.DynamicBuffers.Exec([&dev = gfx.GetDevice()](auto& x) { x.Init(dev); });

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
		U64 levelCount = 0;
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
						const RID rid = it->GetOutputResources().at(k);
						if (rid >= frameBufferDesc.ResourceInfo.size())
							throw ZE_RGC_EXCEPT("Cannot find resource for input [" + input + "], RID out of range [" + std::to_string(rid) + "]!");

						node.AddInputResource(rid);
						Resource::State currentState = node.GetInputState(j);
						auto& lifetime = frameBufferDesc.ResourceLifetimes.at(rid);

						if (lifetime.contains(depLevel))
							AssignState(lifetime.at(depLevel).first, currentState, rid, depLevel);
						else
							lifetime[depLevel] = { currentState, node.GetPassType() };
						break;
					}
					else if (++k == it->GetOutputs().size())
						throw ZE_RGC_EXCEPT("Cannot find source for input [" + input + "]!");
				}
				++j;
			}
			// Check all output resources
			for (U64 j = 0; const RID rid : node.GetOutputResources())
			{
				if (rid >= frameBufferDesc.ResourceInfo.size())
					throw ZE_RGC_EXCEPT("RID out of range [" + std::to_string(rid) + "] for output + [" + node.GetOutputs().at(j) + "]!");

				Resource::State currentState = node.GetOutputState(j);
				auto& lifetime = frameBufferDesc.ResourceLifetimes.at(rid);

				if (lifetime.contains(depLevel))
				{
					AssignState(lifetime.at(depLevel).first, currentState, rid, depLevel);
					if (lifetime.at(depLevel).second != node.GetPassType())
					{
						if ((frameBufferDesc.ResourceInfo.at(rid).Flags & FrameResourceFlags::SimultaneousAccess) == 0)
							throw ZE_RGC_EXCEPT("Resource of output [" + node.GetOutputs().at(j) + "] used on 2 different engines on level ["
								+ std::to_string(depLevel) + "] without SimultaneousAccess flag!");

						lifetime.at(depLevel).second = QueueType::Main;
					}
				}
				else
					lifetime[depLevel] = { currentState, node.GetPassType() };
				++j;
			}
			// Check temporary inner resources
			for (auto& innerBuffer : node.GetInnerBuffers())
			{
				node.AddInnerBufferResource(frameBufferDesc.AddResource(std::move(innerBuffer.Info)));
				frameBufferDesc.ResourceLifetimes.back()[depLevel] = { innerBuffer.InitState, node.GetPassType() };
			}
		}

		// Divide into render levels and find required syncs between them
		// Gfx -> Compute | Compute -> Gfx | Compute -> Compute
		std::vector<std::pair<std::pair<bool, bool>, bool>> requiredSyncs;
		// First bundle always starts at the begining
		std::vector<U64> renderLevelStarts = { 0 };
		for (U64 i = 0; auto& syncs : syncList)
		{
			if (syncs.size())
			{
				const U64 currentLevel = dependencyLevels.at(i);
				U16 renderLevel = 0;
				for (U16 level : renderLevelStarts)
				{
					if (level == currentLevel)
						break;
					++renderLevel;
				}
				if (renderLevel >= renderLevelStarts.size())
				{
					renderLevel = renderLevelStarts.size() - 1;
					renderLevelStarts.emplace_back(currentLevel);
					requiredSyncs.emplace_back(std::make_pair(false, false), false);
				}

				if (nodes.at(i).GetPassType() == QueueType::Main)
					requiredSyncs.at(renderLevel).first.second = true;
				else
					requiredSyncs.at(renderLevel).first.first = true;

				// Check if possible situation when Compute resource will be used on Compute and Gfx queue
				// NOTE: Only possible if resource have SimultaneousAccess flag set
				for (U64 dep : depList.at(i))
				{
					if (nodes.at(dep).GetPassType() == QueueType::Compute)
					{
						// Check whether dependent pass have output resource shared between engines
						for (RID res : nodes.at(dep).GetOutputResources())
						{
							if (frameBufferDesc.ResourceInfo.at(res).Flags & FrameResourceFlags::SimultaneousAccess)
							{
								requiredSyncs.at(renderLevel).second = true;
								break;
							}
						}
						if (requiredSyncs.at(renderLevel).second)
							break;
					}
				}
			}
			++i;
		}
		depList.clear();
		syncList.clear();

		// Create render levels and compute how many pass levels fits in them
		renderLevelCount = renderLevelStarts.size();
		levels = new RenderLevel[renderLevelCount];
		for (U16 i = 1; i < renderLevelCount; ++i)
			levels[i - 1].LevelCount = renderLevelStarts.at(i) - renderLevelStarts.at(i - 1);
		levels[renderLevelStarts.size() - 1].LevelCount = ++levelCount - renderLevelStarts.back();

		// Compute proper syncs between levels
		for (U16 i = 0; const auto& sync : requiredSyncs)
		{
			if (sync.second)
				levels[i].ExitSync = SyncType::ComputeToAll;
			else
			{
				if (sync.first.first)
					levels[i].ExitSync = SyncType::MainToCompute;
				if (sync.first.second)
					levels[i + 1].EnterSync = SyncType::ComputeToMain;
			}
			++i;
		}
		for (U16 i = 0; i < renderLevelCount; ++i)
			frameBufferDesc.RenderLevels.emplace_back(std::make_pair(renderLevelStarts.at(i), levels[i].LevelCount), std::make_pair(levels[i].EnterSync, levels[i].ExitSync));
		requiredSyncs.clear();

		// Gather data about how many passes are on each level
		// Gfx count, compute count | pass list
		std::vector<std::pair<std::pair<U16, U16>, std::vector<std::pair<U64, QueueType>>>> passLevels(levelCount, { { 0, 0 }, {} });
		for (U64 i = 0; const auto& node: nodes)
		{
			ZE_ASSERT(node.GetPassType() != QueueType::Copy, "Incorrect pass type in render graph!");
			auto& level = passLevels.at(dependencyLevels.at(i));
			level.second.emplace_back(i++, node.GetPassType());

			if (node.GetPassType() == QueueType::Main)
				++level.first.first;
			else
				++level.first.second;
		}
		dependencyLevels.clear();

		// Emplace passes into final structure
		passCleaners = new std::array<Ptr<Ptr<PassCleanCallback>>, 2>[renderLevelCount];
		PassDesc* passDescs = new PassDesc[nodes.size()];
		PassCleanCallback* passCleanCallbacks = new PassCleanCallback[nodes.size()];
		for (U16 i = 0; i < renderLevelCount; ++i)
		{
			auto& renderLevel = levels[i];
			auto& renderLevelCleaners = passCleaners[i];
			const U64 levelStart = renderLevelStarts.at(i);

			// Check what passes are on this level
			bool gfxPresent = false, computePresent = false;
			for (U16 j = 0; j < renderLevel.LevelCount; ++j)
			{
				const auto& passLevel = passLevels.at(j + levelStart);

				if (passLevel.first.first)
					gfxPresent = true;
				if (passLevel.first.second)
					computePresent = true;
				if (gfxPresent && computePresent)
					break;
			}
			// Create correct bundles
			ZE_ASSERT(gfxPresent || computePresent, "At least one bundle must be present on each render level!");
			if (gfxPresent)
			{
				renderLevel.Bundles[0] = new std::pair<Ptr<PassDesc>, U16>[renderLevel.LevelCount];
				renderLevelCleaners.at(0) = new Ptr<PassCleanCallback>[renderLevel.LevelCount];
			}
			if (computePresent)
			{
				renderLevel.Bundles[1] = new std::pair<Ptr<PassDesc>, U16>[renderLevel.LevelCount];
				renderLevelCleaners.at(1) = new Ptr<PassCleanCallback>[renderLevel.LevelCount];
			}

			// Populate each level of bundle with passes
			for (U16 j = 0; j < renderLevel.LevelCount; ++j)
			{
				const auto& passLevel = passLevels.at(j + levelStart);
				ZE_ASSERT(passLevel.first.first || passLevel.first.second, "At least one pass must be present on each level!");

				if (renderLevel.Bundles[0])
				{
					renderLevel.Bundles[0][j].second = passLevel.first.first;
					if (passLevel.first.first)
					{
						renderLevel.Bundles[0][j].first = passDescs;
						renderLevelCleaners.at(0)[j] = passCleanCallbacks;
						passDescs += passLevel.first.first;
						passCleanCallbacks += passLevel.first.first;
					}
				}

				if (renderLevel.Bundles[1])
				{
					renderLevel.Bundles[1][j].second = passLevel.first.second;
					if (passLevel.first.second)
					{
						renderLevel.Bundles[1][j].first = passDescs;
						renderLevelCleaners.at(1)[j] = passCleanCallbacks;
						passDescs += passLevel.first.second;
						passCleanCallbacks += passLevel.first.second;
					}
				}

#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
				if (passLevel.first.first > gfxWorkersCount)
					gfxWorkersCount = passLevel.first.first;
				if (passLevel.first.second > computeWorkersCount)
					computeWorkersCount = passLevel.first.second;
#endif
				U16 gfx = 0, compute = 0;
				for (const auto& pass : passLevel.second)
				{
					auto& node = nodes.at(pass.first);
					ZE_ASSERT((node.GetExecuteData() != nullptr) == (node.GetCleanCallback() != nullptr),
						"Optional data and cleaning function must be both provided or neither of them!");
					ZE_ASSERT(node.GetExecuteCallback(), "Empty execution callback!");

					auto& desc = pass.second == QueueType::Main ?
						renderLevel.Bundles[0][j].first[gfx] : renderLevel.Bundles[1][j].first[compute];
					desc.Execute = node.GetExecuteCallback();
					desc.Data.OptData = node.GetExecuteData();
					desc.Data.Buffers = node.GetNodeRIDs();

					(pass.second == QueueType::Main ?
						renderLevelCleaners.at(0)[j][gfx++] :
						renderLevelCleaners.at(1)[j][compute++]) = node.GetCleanCallback();
				}
			}
		}
		renderLevelStarts.clear();
		passLevels.clear();

#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
		if (gfxWorkersCount > 1)
		{
			workerThreadsGfx = new std::pair<std::thread, ChainPool<CommandList>>[--gfxWorkersCount];
			for (U16 i = 0; i < gfxWorkersCount; ++i)
				workerThreadsGfx[i].second.Exec([&dev](auto& x) { x.Init(dev, CommandType::All); });
		}
		else
			gfxWorkersCount = 0;
		if (computeWorkersCount > 1)
		{
			workerThreadsCompute = new std::pair<std::thread, ChainPool<CommandList>>[--computeWorkersCount];
			for (U16 i = 0; i < computeWorkersCount; ++i)
				workerThreadsCompute[i].second.Exec([&dev](auto& x) { x.Init(dev, CommandType::Compute); });
		}
		else
			computeWorkersCount = 0;
#endif

		frameBufferDesc.ComputeWorkflowTransitions(levelCount);
		execData.Buffers.Init(gfx, frameBufferDesc);
		for (U16 i = 0; const auto& levelInfo : frameBufferDesc.RenderLevels)
		{
			if (levelInfo.second.first != levels[i].EnterSync)
			{
				ZE_ASSERT(levelInfo.second.first == SyncType::MainToAll, "Unsupported sync type!");
				levels[i].EnterSync = levelInfo.second.first;
			}
			++i;
		}

		// Create shared gfx states
		if (buildData.PipelineStates.size())
		{
			execData.SharedStates = new Resource::PipelineStateGfx[buildData.PipelineStates.size()];
			for (U64 i = 0; const auto& state : buildData.PipelineStates)
				execData.SharedStates[i++].Init(gfx.GetDevice(), state.second.second.first, execData.Bindings.GetSchema(state.second.first));
		}
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
		if (workerThreadsGfx)
			workerThreadsGfx.DeleteArray();
		if (workerThreadsCompute)
			workerThreadsCompute.DeleteArray();
#endif
		if (levels)
		{
			PassDesc* passes;
			if (levels[0].Bundles[0] && levels[0].Bundles[0][0].first)
				passes = levels[0].Bundles[0][0].first;
			else
				passes = levels[0].Bundles[1][0].first;
			PassCleanCallback* cleaners;
			if (passCleaners[0].at(0) && passCleaners[0].at(0)[0])
				cleaners = passCleaners[0].at(0)[0];
			else
				cleaners = passCleaners[0].at(1)[0];

			for (U16 i = 0; i < renderLevelCount; ++i)
			{
				auto& level = levels[i];
				auto levelCleaners = passCleaners[i];
				for (U8 bundleType = 0; bundleType < 2; ++bundleType)
				{
					if (!level.Bundles[bundleType])
						continue;

					auto bundle = level.Bundles[bundleType];
					auto bundleCleanres = levelCleaners[bundleType];
					for (U16 j = 0; j < level.LevelCount; ++j)
					{
						auto& passLevel = bundle[j];
						auto passLevelCleaners = bundleCleanres[j];
						for (U16 k = 0; k < passLevel.second; ++k)
						{
							auto& pass = passLevel.first[k];
							if (pass.Data.Buffers)
								pass.Data.Buffers.DeleteArray();
							if (pass.Data.OptData)
								passLevelCleaners[k](pass.Data.OptData);
						}
					}
					bundle.DeleteArray();
					bundleCleanres.DeleteArray();
				}
			}
			if (passes)
				delete[] passes;
			if (cleaners)
				delete[] cleaners;
			levels.DeleteArray();
			passCleaners.DeleteArray();
		}
		if (execData.SharedStates)
			execData.SharedStates.DeleteArray();
	}

	void RenderGraph::Execute(Graphics& gfx)
	{
		Device& dev = gfx.GetDevice();
		CommandList* lists[2] = { &gfx.GetMainList(), &gfx.GetComputeList() };

		//switch (Settings::GetGfxApi())
		//{
		//default:
		//	ZE_ASSERT(false, "Unhandled enum value!");
		//case GfxApiType::DX11:
		//case GfxApiType::OpenGL:
		//{
		//}
		//case GfxApiType::DX12:
		//case GfxApiType::Vulkan:
		//{
		//	break;
		//}
		//}

		gfx.WaitForFrame();
		execData.DynamicBuffers.Get().StartFrame(dev);
		execData.DynamicBuffers.Get().Alloc(dev, execData.DynamicData, dynamicDataSize);
		execData.Buffers.SwapBackbuffer(dev, gfx.GetSwapChain());

		for (auto& cl : lists)
			cl->Reset(dev);
#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
		for (U64 i = 0; i < workersCount; ++i)
			workerThreads[i].second.Get().Reset(dev);
#endif
		for (U16 i = 0; i < renderLevelCount; ++i)
		{
			auto& level = levels[i];
			ZE_ASSERT(level.Bundles[0] || level.Bundles[1], "At least one bundle must be present at each rendering level!");

			// Enter sync required for level (if need to wait for Compute engine resources to perform transitions)
			switch (level.EnterSync)
			{
			case SyncType::ComputeToMain:
			{
				dev.WaitMainFromCompute(dev.SetComputeFence());
				break;
			}
			case SyncType::MainToAll:
			{
				dev.WaitMainFromCompute(dev.SetComputeFence());
				auto& cl = *lists[0];

				cl.Open(dev);
				ZE_DRAW_TAG_BEGIN(cl, (L"Cross engine wrapping at render level " + std::to_wstring(i + 1)).c_str(), PixelVal::Red);
				execData.Buffers.SyncedEntryTransitions(cl, i);
				ZE_DRAW_TAG_END(cl);
				cl.Close(dev);
				dev.ExecuteMain(cl);

				dev.WaitComputeFromMain(dev.SetMainFence());
				break;
			}
			default:
				ZE_ASSERT(false, "Incorret enum value for enter sync context!");
			case SyncType::None:
				break;
			}

			for (U8 bundleType = 0; bundleType < 2; ++bundleType)
			{
				if (!level.Bundles[bundleType])
					continue;

				const QueueType queue = bundleType ? QueueType::Compute : QueueType::Main;
				auto& cl = *lists[bundleType];
				cl.Open(dev);
				ZE_DRAW_TAG_BEGIN(cl, (L"Render level " + std::to_wstring(i + 1) + (bundleType ? L" [Compute]" : L" [GFX]")).c_str(), PixelVal::White);
				execData.Buffers.EntryTransitions(cl, queue, i);

				auto bundle = level.Bundles[bundleType];
				for (U16 j = 0; j < level.LevelCount; ++j)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Pass level " + std::to_wstring(j + 1)).c_str(), PixelVal::White);
					auto& passLevel = bundle[j];

					for (U16 k = 0; k < passLevel.second; ++k)
					{
						auto& pass = passLevel.first[k];
						pass.Execute(dev, cl, execData, pass.Data);
					}
					execData.Buffers.ExitTransitions(cl, queue, i, j);
					ZE_DRAW_TAG_END(cl);
				}

				ZE_DRAW_TAG_END(cl);
				cl.Close(dev);
				if (bundleType)
					dev.ExecuteCompute(cl);
				else
					dev.ExecuteMain(cl);
			}

			// Exit sync if barriers reference cross-engine resources
			switch (level.ExitSync)
			{
			case SyncType::MainToCompute:
			{
				dev.WaitComputeFromMain(dev.SetMainFence());
				break;
			}
			case SyncType::ComputeToAll:
			{
				dev.WaitMainFromCompute(dev.SetComputeFence());
				auto& cl = *lists[0];

				cl.Open(dev);
				ZE_DRAW_TAG_BEGIN(cl, (L"Cross engine transition at render level " + std::to_wstring(i + 1)).c_str(), PixelVal::Red);
				execData.Buffers.SyncedExitTransitions(cl, i);
				ZE_DRAW_TAG_END(cl);
				cl.Close(dev);
				dev.ExecuteMain(cl);

				dev.WaitComputeFromMain(dev.SetMainFence());
				break;
			}
			default:
				ZE_ASSERT(false, "Incorret enum value for exit sync context!");
			case SyncType::None:
				break;
			}

			//			auto& level = passes[i];
			//			if (level.second)
			//			{
			//#ifdef _ZE_RENDER_GRAPH_SINGLE_THREAD
			//				for (U64 j = 0; j < level.second; ++j)
			//					ExecuteThreadSync(dev, mainList, level.first[j]);
			//#else
			//				U64 workersDispatch = level.second - 1;
			//				ZE_ASSERT(workersCount >= workersDispatch, "Insufficient number of workers!");
			//				for (U64 j = 0; j < workersDispatch; ++j)
			//					workerThreads[j].first = { &RenderGraph::ExecuteThreadSync, this, std::ref(dev), std::ref(workerThreads[j].second.Get()), std::ref(level.first[j]) };
			//				ExecuteThreadSync(dev, mainList, level.first[workersDispatch]);
			//				for (U64 j = 0; j < workersDispatch; ++j)
			//					workerThreads[j].first.join();
			//#endif
			//			}
		}
	}
}