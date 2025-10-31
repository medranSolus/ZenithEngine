#include "GFX/Pipeline/RenderPass/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::DearImGui
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for DearImGui initialization formats!");
		return Initialize(dev, buildData, formats.at(0));
	}

	PassDesc GetDesc(PixelFormat formatRT) noexcept
	{
		PassDesc desc{ Base(CorePassType::DearImGui) };
		desc.InitializeFormats.reserve(1);
		desc.InitializeFormats.emplace_back(formatRT);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Clean = Clean;
		return desc;
	}

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		syncStatus.SyncMain(dev);
		GUI::ImGuiManager::DestroyRenderData(dev, reinterpret_cast<ExecuteData*>(data)->GuiData);
		delete reinterpret_cast<ExecuteData*>(data);
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT)
	{
		ExecuteData* passData = new ExecuteData;
		passData->GuiData = GUI::ImGuiManager::CreateRenderData(dev, formatRT);
		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Resources.CastConst<Resources>();

		if (Settings::IsEnabledImGui())
		{
			ZE_PERF_GUARD("ImGui");

			ZE_DRAW_TAG_BEGIN(dev, cl, "ImGui", PixelVal::Cobalt);
			renderData.Buffers.BeginRaster(cl, ids.Output);

			GUI::ImGuiManager::RunRender(cl);

			renderData.Buffers.EndRaster(cl);
			ZE_DRAW_TAG_END(dev, cl);
			return true;
		}
		return false;
	}
}