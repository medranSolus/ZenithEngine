#include "GFX/Pipeline/RenderGraph.h"

namespace ZE::GFX::Pipeline
{
	void RenderGraph::Execute(Graphics& gfx)
	{
		CommandList& asyncList = asyncListChain.Get();
		CommandList& mainList = gfx.GetMainList();
		Device& dev = gfx.GetDevice();
		execData.Buffers.SwapBackbuffer(dev, gfx.GetSwapChain());

		// If needed do an update
		switch (update)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case PendingUpdate::None:
			break;
		case PendingUpdate::Soft:
			break;
		case PendingUpdate::Hard:
			break;
		}

		// TODO: Single threaded method only for now
		for (U32 i = 0; i < execGroupCount; ++i)
		{
			auto& mainGroup = passExecGroups[i].at(0);
			auto& asyncGroup = passExecGroups[i].at(1);

			if (mainGroup.PassGroupCount)
			{
				ZE_DRAW_TAG_BEGIN_MAIN(dev, "Main execution group, level " + std::to_string(i + 1), PixelVal::White);
				if (mainGroup.QueueWait)
					dev.WaitMainFromCompute(mainGroup.WaitFence);

				mainList.Open(dev);
				for (U32 j = 0; j < mainGroup.PassGroupCount; ++j)
				{
					auto& parallelGroup = mainGroup.PassGroups[j];
					if (parallelGroup.StartBarriers.size())
						execData.Buffers.Barrier(mainList, parallelGroup.StartBarriers.data(), Utils::SafeCast<U32>(parallelGroup.StartBarriers.size()));

					for (U32 k = 0; k < parallelGroup.PassCount; ++k)
						parallelGroup.Passes[k].Exec(dev, mainList, execData, parallelGroup.Passes[k].Data);
				}
				if (mainGroup.EndBarriers.size())
					execData.Buffers.Barrier(mainList, mainGroup.EndBarriers.data(), Utils::SafeCast<U32>(mainGroup.EndBarriers.size()));
				mainList.Close(dev);

				if (mainGroup.SignalFence)
					*mainGroup.SignalFence = dev.SetMainFence();
				ZE_DRAW_TAG_END_MAIN(dev);
			}

			if (asyncGroup.PassGroupCount)
			{
				ZE_DRAW_TAG_BEGIN_COMPUTE(dev, "Async execution group, level " + std::to_string(i + 1), PixelVal::White);
				if (asyncGroup.QueueWait)
					dev.WaitComputeFromMain(asyncGroup.WaitFence);

				asyncList.Open(dev);
				for (U32 j = 0; j < asyncGroup.PassGroupCount; ++j)
				{
					auto& parallelGroup = asyncGroup.PassGroups[j];
					if (parallelGroup.StartBarriers.size())
						execData.Buffers.Barrier(asyncList, parallelGroup.StartBarriers.data(), Utils::SafeCast<U32>(parallelGroup.StartBarriers.size()));

					for (U32 k = 0; k < parallelGroup.PassCount; ++k)
						parallelGroup.Passes[k].Exec(dev, asyncList, execData, parallelGroup.Passes[k].Data);
				}
				if (asyncGroup.EndBarriers.size())
					execData.Buffers.Barrier(asyncList, asyncGroup.EndBarriers.data(), Utils::SafeCast<U32>(asyncGroup.EndBarriers.size()));
				asyncList.Close(dev);

				if (asyncGroup.SignalFence)
					*asyncGroup.SignalFence = dev.SetComputeFence();
				ZE_DRAW_TAG_END_COMPUTE(dev);
			}
		}
	}

	void RenderGraph::ShowDebugUI() noexcept
	{
		ImGui::BeginChild("##render_graph", { 100.0f, 100.0f }, ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		ImGui::EndChild();
	}

	void RenderGraph::Free(Device& dev) noexcept
	{
		execGroupCount = 0;
		passExecGroups = nullptr;
		asyncListChain.Exec([&dev](CommandList& x) { x.Free(dev); });
		execData.Buffers.Free(dev);
		execData.Bindings.Free(dev);
		execData.SettingsBuffer.Free(dev);
	}
}