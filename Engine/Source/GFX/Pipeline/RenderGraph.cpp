#include "GFX/Pipeline/RenderGraph.h"

namespace ZE::GFX::Pipeline
{
	void RenderGraph::Execute(Graphics& gfx)
	{
		CommandList& mainList = gfx.GetMainList();
		Device& dev = gfx.GetDevice();

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
			auto& mainGroup = passGroups[i].at(0);
			auto& asyncGroup = passGroups[i].at(1);

			if (mainGroup.Count)
			{
				ZE_DRAW_TAG_BEGIN_MAIN(dev, "Main execution group, level " + std::to_string(i + 1), PixelVal::White);
				if (mainGroup.QueueWait)
					dev.WaitMainFromCompute(mainGroup.WaitFence);

				mainList.Open(dev);
				for (U32 j = 0; j < mainGroup.Count; ++j)
				{
					auto& parallelGroup = mainGroup.PassGroups[j];
					for (U32 k = 0; k < parallelGroup.Count; ++k)
					{
						// TODO: resource barriers - gather barriers needed between passes and execute them together
						parallelGroup.Passes[k].first(dev, mainList, execData, parallelGroup.Passes[k].second);
					}
				}
				// TODO: last exec group with some last transitions for backbuffer and others, have to be main group
				mainList.Close(dev);

				if (mainGroup.SignalFence)
					*mainGroup.SignalFence = dev.SetMainFence();
				ZE_DRAW_TAG_END_MAIN(dev);
			}

			if (asyncGroup.Count)
			{
				ZE_DRAW_TAG_BEGIN_COMPUTE(dev, "Async execution group, level " + std::to_string(i + 1), PixelVal::White);
				if (asyncGroup.QueueWait)
					dev.WaitComputeFromMain(asyncGroup.WaitFence);

				asyncList.Open(dev);
				for (U32 j = 0; j < asyncGroup.Count; ++j)
				{
					auto& parallelGroup = asyncGroup.PassGroups[j];
					for (U32 k = 0; k < parallelGroup.Count; ++k)
					{
						// TODO: resource barriers - gather barriers needed between passes and execute them together
						parallelGroup.Passes[k].first(dev, asyncList, execData, parallelGroup.Passes[k].second);
					}
				}
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
}