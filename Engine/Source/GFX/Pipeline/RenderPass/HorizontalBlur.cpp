#include "GFX/Pipeline/RenderPass/HorizontalBlur.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::HorizontalBlur
{
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Direction
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Blur texture
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"BlurPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		ZE_PSO_SET_NAME(psoDesc, "HorizontalBlur");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		cl.Open(dev, data.State);
		ZE_DRAW_TAG_BEGIN(dev, cl, L"Outline Horizontal Blur", Pixel(0xF3, 0xEA, 0xAF));
		ctx.BindingSchema.SetGraphics(cl);
		renderData.Buffers.InitRTV(cl, ids.RenderTarget);

		Resource::Constant<U32> direction(dev, false);
		direction.Bind(cl, ctx);
		renderData.Buffers.SetSRV(cl, ctx, ids.Outline);
		renderData.SettingsBuffer.Bind(cl, ctx);
		renderData.Buffers.SetRTV(cl, ids.RenderTarget);
		cl.DrawFullscreen(dev);

		ZE_DRAW_TAG_END(dev, cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}