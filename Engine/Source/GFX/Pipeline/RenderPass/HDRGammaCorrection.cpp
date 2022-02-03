#include "GFX/Pipeline/RenderPass/HDRGammaCorrection.h"

namespace ZE::GFX::Pipeline::RenderPass::HDRGammaCorrection
{
	Data* Setup(Device& dev, RendererBuildData& buildData, PixelFormat outputFormat)
	{
		Data* passData = new Data;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
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

	void Execute(RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		Data& data = *reinterpret_cast<Data*>(passData.OptData);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		renderData.CL.Open(renderData.Dev, data.State);
		ctx.BindingSchema.SetGraphics(renderData.CL);

		renderData.Buffers.SetSRV(renderData.CL, ctx, ids.Scene);
		renderData.EngineData.Bind(renderData.CL, ctx);
		renderData.Buffers.SetRTV(renderData.CL, ids.RenderTarget);
		renderData.CL.DrawFullscreen(renderData.Dev);

		renderData.CL.Close(renderData.Dev);
		renderData.Dev.ExecuteMain(renderData.CL);
	}
}