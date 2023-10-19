#include "GFX/Pipeline/RenderPass/LightCombine.h"

namespace ZE::GFX::Pipeline::RenderPass::LightCombine
{
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat outputFormat)
	{
		ExecuteData* passData = new ExecuteData;
		const bool isAO = Settings::GetAOType() != AOType::None;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // GBuff color
		desc.AddRange({ 2, 1, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Light map, specular map
		if (isAO)
			desc.AddRange({ 1, 3, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // SSAO
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, isAO ? "LightCombinePS" : "LightCombineNoAOPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = outputFormat;
		ZE_PSO_SET_NAME(psoDesc, isAO ? "LightCombine" : "LightCombineNoAO");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Light Combine");

		RID gbufferColor;
		RID lightColor;
		RID rtv;
		RID ssao;
		if (Settings::GetAOType() != AOType::None)
		{
			ResourcesAO ids = *passData.Buffers.CastConst<ResourcesAO>();
			gbufferColor = ids.GBufferColor;
			lightColor = ids.LightColor;
			rtv = ids.RenderTarget;
			ssao = ids.SSAO;
		}
		else
		{
			ResourcesNoAO ids = *passData.Buffers.CastConst<ResourcesNoAO>();
			gbufferColor = ids.GBufferColor;
			lightColor = ids.LightColor;
			rtv = ids.RenderTarget;
			ssao = 0;
		}
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		cl.Open(dev, data.State);
		ZE_DRAW_TAG_BEGIN(dev, cl, "LightCombine", Pixel(0xFF, 0xFF, 0x9F));
		ctx.BindingSchema.SetGraphics(cl);

		renderData.Buffers.InitRTV(cl, rtv);
		renderData.Buffers.SetSRV(cl, ctx, gbufferColor);
		renderData.Buffers.SetSRV(cl, ctx, lightColor);
		if (ssao)
			renderData.Buffers.SetSRV(cl, ctx, ssao);
		renderData.SettingsBuffer.Bind(cl, ctx);
		renderData.Buffers.SetRTV(cl, rtv);
		cl.DrawFullscreen(dev);

		ZE_DRAW_TAG_END(dev, cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}