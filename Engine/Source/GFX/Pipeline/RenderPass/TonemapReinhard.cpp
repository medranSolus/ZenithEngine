#include "GFX/Pipeline/RenderPass/TonemapReinhard.h"

namespace ZE::GFX::Pipeline::RenderPass::TonemapReinhard
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for TonemapReinhard initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	PassDesc GetDesc(PixelFormat outputFormat) noexcept
	{
		PassDesc desc{ Base(CorePassType::TonemapReinhard) };
		desc.InitializeFormats.emplace_back(outputFormat);
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
		execData->State.Free(dev);
		delete execData;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Frame
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, "ReinhardPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = outputFormat;
		ZE_PSO_SET_NAME(psoDesc, "TonemapReinhard");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("TonemapReinhard");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_DRAW_TAG_BEGIN(dev, cl, "TonemapReinhard", PixelVal::White);
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.Scene);
		cl.DrawFullscreen(dev);

		renderData.Buffers.EndRaster(cl);
		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}
}