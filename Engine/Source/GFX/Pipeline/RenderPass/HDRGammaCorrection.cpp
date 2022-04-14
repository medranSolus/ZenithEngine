#include "GFX/Pipeline/RenderPass/HDRGammaCorrection.h"

namespace ZE::GFX::Pipeline::RenderPass::HDRGammaCorrection
{
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat outputFormat)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Frame
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"HDRGammaPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = outputFormat;
		ZE_PSO_SET_NAME(psoDesc, "HDRGammaCorrection");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		cl.Open(dev, data.State);
		ZE_DRAW_TAG_BEGIN(cl, L"HDRGammaCorrection", PixelVal::White);
		ctx.BindingSchema.SetGraphics(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.Scene);
		renderData.SettingsBuffer.Bind(cl, ctx);
		renderData.Buffers.SetRTV(cl, ids.RenderTarget);
		cl.DrawFullscreen(dev);

		ZE_DRAW_TAG_END(cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}