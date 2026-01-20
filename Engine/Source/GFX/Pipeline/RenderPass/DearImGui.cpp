#include "GFX/Pipeline/RenderPass/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::DearImGui
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 2, "Incorrect size for DearImGui initialization formats!");
		return Initialize(dev, buildData, formats.at(0), formats.at(1));
	}

	PassDesc GetDesc(PixelFormat formatUI, PixelFormat formatRT) noexcept
	{
		PassDesc desc{ Base(CorePassType::DearImGui) };
		desc.InitializeFormats.reserve(1);
		desc.InitializeFormats.emplace_back(formatUI);
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
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		GUI::ImGuiManager::DestroyRenderData(dev, execData->GuiData);
		execData->State.Free(dev);
		delete execData;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatUI, PixelFormat formatRT)
	{
		ExecuteData* passData = new ExecuteData;
		passData->GuiData = GUI::ImGuiManager::CreateRenderData(dev, formatUI);

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // UI
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, "GammaRemovePS", buildData.ShaderCache);
		psoDesc.Blender = Resource::BlendType::Normal;
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		ZE_PSO_SET_NAME(psoDesc, "ImGuiGammaRemove");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));


		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		if (Settings::IsEnabledImGui())
		{
			ZE_PERF_GUARD("ImGui");
			Resources ids = *passData.Resources.CastConst<Resources>();
			ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

			ZE_DRAW_TAG_BEGIN(dev, cl, "ImGui", PixelVal::Cobalt);

			renderData.Buffers.ClearRTV(cl, ids.UI, ColorF4(0.0f, 0.0f, 0.0f, 0.0f));

			// Render UI to temp buffer
			renderData.Buffers.BeginRaster(cl, ids.UI);
			GUI::ImGuiManager::RunRender(cl);
			renderData.Buffers.EndRaster(cl);

			// UI transition to SRV
			BarrierTransition barrier = {};
			barrier.Resource = ids.UI;
			barrier.LayoutBefore = TextureLayout::RenderTarget;
			barrier.LayoutAfter = TextureLayout::ShaderResource;
			barrier.AccessBefore = Base(ResourceAccess::RenderTarget);
			barrier.AccessAfter = Base(ResourceAccess::ShaderResource);
			barrier.StageBefore = Base(StageSync::RenderTarget);
			barrier.StageAfter = Base(StageSync::PixelShading);
			renderData.Buffers.Barrier(cl, barrier);

			// Apply UI and remove gamma in the process
			renderData.Buffers.BeginRaster(cl, ids.Output);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			ctx.BindingSchema.SetGraphics(cl);
			data.State.Bind(cl);

			renderData.Buffers.SetSRV(cl, ctx, ids.UI);
			cl.DrawFullscreen(dev);
			renderData.Buffers.EndRaster(cl);
			
			// UI back to RTV
			barrier.LayoutBefore = TextureLayout::ShaderResource;
			barrier.LayoutAfter = TextureLayout::RenderTarget;
			barrier.AccessBefore = Base(ResourceAccess::ShaderResource);
			barrier.AccessAfter = Base(ResourceAccess::RenderTarget);
			barrier.StageBefore = Base(StageSync::PixelShading);
			barrier.StageAfter = Base(StageSync::RenderTarget);
			renderData.Buffers.Barrier(cl, barrier);

			ZE_DRAW_TAG_END(dev, cl);
			return true;
		}
		return false;
	}
}