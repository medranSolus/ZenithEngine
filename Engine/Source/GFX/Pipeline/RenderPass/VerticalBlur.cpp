#include "GFX/Pipeline/RenderPass/VerticalBlur.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::VerticalBlur
{
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"BlurPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::StencilMask;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.Blender = Resource::BlendType::Normal;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		psoDesc.FormatDS = formatDS;
		ZE_PSO_SET_NAME(psoDesc, "VerticalBlur");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		cl.Open(dev, data.State);
		ZE_DRAW_TAG_BEGIN(cl, L"Outline Vertical Blur", Pixel(0xFD, 0xEF, 0xB2));
		ctx.BindingSchema.SetGraphics(cl);

		data.State.SetStencilRef(cl, 0xFF);
		Resource::Constant<U32> direction(dev, true);
		direction.Bind(cl, ctx);
		renderData.Buffers.SetSRV(cl, ctx, ids.OutlineBlur);
		renderData.SettingsBuffer.Bind(cl, ctx);
		renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.DepthStencil);
		cl.DrawFullscreen(dev);

		ZE_DRAW_TAG_END(cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}