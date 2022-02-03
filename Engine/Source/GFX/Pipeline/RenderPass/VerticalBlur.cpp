#include "GFX/Pipeline/RenderPass/VerticalBlur.h"

namespace ZE::GFX::Pipeline::RenderPass::VerticalBlur
{
	Data* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS)
	{
		Data* passData = new Data;

		bool direction = true;
		dev.StartUpload();
		passData->Direction.Init(dev, &direction, sizeof(bool), false);
		dev.FinishUpload();

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Can be Constant, implement that, maybe flag and new update method
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

	void Execute(RendererExecuteData& renderData, PassData& passData)
	{
		return;
		Resources ids = *passData.Buffers.CastConst<Resources>();
		Data& data = *reinterpret_cast<Data*>(passData.OptData);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		renderData.CL.Open(renderData.Dev, data.State);
		ctx.BindingSchema.SetGraphics(renderData.CL);

		data.Direction.Bind(renderData.CL, ctx);
		renderData.Buffers.SetSRV(renderData.CL, ctx, ids.OutlineBlur);
		renderData.EngineData.Bind(renderData.CL, ctx);
		renderData.Buffers.SetOutput(renderData.CL, ids.RenderTarget, ids.DepthStencil);
		renderData.CL.Draw(renderData.Dev, 0);

		renderData.CL.Close(renderData.Dev);
		renderData.Dev.ExecuteMain(renderData.CL);
	}
}