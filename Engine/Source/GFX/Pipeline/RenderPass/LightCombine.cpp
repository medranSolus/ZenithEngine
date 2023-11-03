#include "GFX/Pipeline/RenderPass/LightCombine.h"

namespace ZE::GFX::Pipeline::RenderPass::LightCombine
{
	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->StateAO.Free(dev);
		execData->StateNoAO.Free(dev);
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat outputFormat)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // GBuff color
		desc.AddRange({ 2, 1, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Light map, specular map
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndexNoAO = buildData.BindingLib.AddDataBinding(dev, desc);

		desc.Ranges.resize(2);
		desc.Samplers.clear();
		desc.AddRange({ 1, 3, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // SSAO
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndexAO = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = outputFormat;
		psoDesc.SetShader(dev, psoDesc.PS, "LightCombinePS_A", buildData.ShaderCache);
		ZE_PSO_SET_NAME(psoDesc, "LightCombinePS_A");
		passData->StateAO.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndexAO));

		psoDesc.SetShader(dev, psoDesc.PS, "LightCombinePS", buildData.ShaderCache);
		ZE_PSO_SET_NAME(psoDesc, "LightCombine");
		passData->StateNoAO.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndexNoAO));

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Light Combine");
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		const bool isAO = ids.SSAO != INVALID_RID;
		Binding::Context ctx{ renderData.Bindings.GetSchema(isAO ? data.BindingIndexAO : data.BindingIndexNoAO) };

		cl.Open(dev, isAO ? data.StateAO : data.StateNoAO);
		ZE_DRAW_TAG_BEGIN(dev, cl, "LightCombine", Pixel(0xFF, 0xFF, 0x9F));
		ctx.BindingSchema.SetGraphics(cl);

		renderData.Buffers.InitRTV(cl, ids.RenderTarget);
		renderData.Buffers.SetSRV(cl, ctx, ids.GBufferColor);
		renderData.Buffers.SetSRV(cl, ctx, ids.LightColor);
		if (isAO)
			renderData.Buffers.SetSRV(cl, ctx, ids.SSAO);
		renderData.SettingsBuffer.Bind(cl, ctx);
		renderData.Buffers.SetRTV(cl, ids.RenderTarget);
		cl.DrawFullscreen(dev);

		ZE_DRAW_TAG_END(dev, cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}