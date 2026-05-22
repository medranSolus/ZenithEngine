#include "GFX/Pipeline/RenderPass/HBAO.h"
#include "GFX/External/Error.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::HBAO
{
	static Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, PassExecuteData* passData, const std::vector<PixelFormat>& formats) noexcept
	{
		return Update(dev, buildData, *static_cast<ExecuteData*>(passData));
	}

	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) noexcept
	{
		return Initialize(dev, buildData);
	}

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::HBAO) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.DebugUI = DebugUI;
		return desc;
	}

	Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData) noexcept
	{
		if (passData.RenderSize != Settings::RenderSize)
		{
			passData.RenderSize = Settings::RenderSize;
			ZE_HBAO_LOG_RET_FAILED_EXPECT(passData.Ctx.CreateResources(dev, passData.Params, passData.RenderSize), "Error recreating resources for the HBAO+!");

			return UpdateOperation::InternalOnly;
		}
		return UpdateOperation::NoUpdate;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();

		ZE_EXPECT_RET_FAILED(passData->Ctx, External::HbaoCtx::Create(dev));
		passData->RenderSize = Settings::RenderSize;
		ZE_HBAO_LOG_RET_FAILED_EXPECT(passData->Ctx.CreateResources(dev, passData->Params, passData->RenderSize), "Error preparing resources for the HBAO+!");

		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		ZE_PERF_GUARD("HBAO");

		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

		ZE_DRAW_TAG_BEGIN(dev, cl, "HBAO", Pixel(0x76, 0xB9, 0x00));

		ZE_HBAO_LOG_RET_FAILED_EXPECT(data.Ctx.Render(dev, renderData.Buffers, data.Params, ids.Depth, ids.Normal, ids.AO), "Error performing HBAO+!");

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		if (ImGui::CollapsingHeader("HBAO+"))
		{
			ExecuteData& execData = *static_cast<ExecuteData*>(data);

			ImGui::Text("Version %u.%u.%u", GFSDK_SSAO_Version{}.Major, GFSDK_SSAO_Version{}.Minor, GFSDK_SSAO_Version{}.Branch);

			ImGui::NewLine();
		}
	}
}