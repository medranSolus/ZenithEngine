#include "GFX/Pipeline/RenderPass/LightCombine.h"

namespace ZE::GFX::Pipeline::RenderPass::LightCombine
{
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat outputFormat)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 2, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // SSAO, GBuff color
		desc.AddRange({ 2, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Light map, specular map
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"LightCombinePS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = outputFormat;
		ZE_PSO_SET_NAME(psoDesc, "LightCombine");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		ZE_DRAW_TAG_BEGIN(cl, L"LightCombine", Pixel(0xFF, 0xFF, 0x9F));
		data.State.Bind(cl);
		ctx.BindingSchema.SetGraphics(cl);

		renderData.Buffers.InitRTV(cl, ids.RenderTarget);
		renderData.Buffers.SetSRV(cl, ctx, ids.SSAO);
		renderData.Buffers.SetSRV(cl, ctx, ids.LightColor);
		renderData.SettingsBuffer.Bind(cl, ctx);
		renderData.Buffers.SetRTV(cl, ids.RenderTarget);

		cl.DrawFullscreen(dev);
		ZE_DRAW_TAG_END(cl);
	}
}