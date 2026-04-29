#include "GFX/Pipeline/RenderPass/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::DearImGui
{
	static Expected<std::unique_ptr<PassExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) noexcept
	{
		ZE_ASSERT(formats.size() == 2, "Incorrect size for DearImGui initialization formats!");
		return Initialize(dev, buildData, formats.at(0), formats.at(1));
	}

	PassDesc GetDesc(PixelFormat formatUI, PixelFormat formatRT) noexcept
	{
		PassDesc desc{ Base(CorePassType::DearImGui) };
		desc.InitializeFormats.reserve(2);
		desc.InitializeFormats.emplace_back(formatUI);
		desc.InitializeFormats.emplace_back(formatRT);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		return desc;
	}

	Expected<std::unique_ptr<ExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatUI, PixelFormat formatRT) noexcept
	{
		auto passData = std::make_unique<ExecuteData>();
		ZE_EXPECT_RET_FAILED(passData->GuiData, ImGuiBackendData::Create(dev, formatUI));

		Binding::SchemaDesc desc = {};
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // UI
		desc.AppendSamplers(buildData.Samplers);
		ZE_EXPECT_RET_FAILED(passData->BindingIndex, buildData.BindingLib.AddDataBinding(dev, desc));

		Resource::PipelineStateDesc psoDesc;
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache));
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.PS, "GammaRemovePS", buildData.ShaderCache));
		psoDesc.Blender = Resource::BlendType::Normal;
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		ZE_PSO_SET_NAME(psoDesc, "ImGuiGammaRemove");
		ZE_EXPECT_RET_FAILED(passData->State, Resource::PipelineStateGfx::Create(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex)));

		return passData;
	}

	Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		if (Settings::IsEnabledImGui())
		{
			ZE_PERF_GUARD("ImGui");
			Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
			ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

			ZE_DRAW_TAG_BEGIN(dev, cl, "ImGui", PixelVal::Cobalt);

			renderData.Buffers.ClearRTV(cl, ids.UI, ColorF4(0.0f, 0.0f, 0.0f, 0.0f));

			// Render UI to temp buffer
			renderData.Buffers.BeginRaster(cl, ids.UI);
			data.GuiData.RunRender(cl);
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
		}
		return {};
	}
}