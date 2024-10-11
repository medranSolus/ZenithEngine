#include "GFX/Pipeline/RenderPass/HDRGammaCorrection.h"

namespace ZE::GFX::Pipeline::RenderPass::HDRGammaCorrection
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for HDRGammaCorrection initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	PassDesc GetDesc(PixelFormat outputFormat) noexcept
	{
		PassDesc desc{ Base(CorePassType::HDRGammaCorrection) };
		desc.InitializeFormats.emplace_back(outputFormat);
		desc.Init = Initialize;
		desc.Execute = Execute;
		desc.Clean = Clean;
		return desc;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Frame
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, "HDRGammaPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = outputFormat;
		ZE_PSO_SET_NAME(psoDesc, "HDRGammaCorrection");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("HDRGammaCorrection");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_DRAW_TAG_BEGIN(dev, cl, "HDRGammaCorrection", PixelVal::White);
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.Scene);
		renderData.SettingsBuffer.Bind(cl, ctx);
		cl.DrawFullscreen(dev);

		renderData.Buffers.EndRaster(cl);
		ZE_DRAW_TAG_END(dev, cl);
	}
}