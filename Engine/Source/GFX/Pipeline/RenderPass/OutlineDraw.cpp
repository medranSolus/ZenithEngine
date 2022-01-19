#include "GFX/Pipeline/RenderPass/OutlineDraw.h"

namespace ZE::GFX::Pipeline::RenderPass::OutlineDraw
{
	Data* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS)
	{
		Data* passData = new Data;

		Float3 solidColor = { 1.0f, 1.0f, 0.0f };
		dev.StartUpload();
		passData->Color.Init(dev, &solidColor, sizeof(Float3), false);
		dev.FinishUpload();

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Can be Constant, implement that, maybe flag and new update method
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"SolidVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"SolidPS", buildData.ShaderCache);
		psoDesc.Stencil = Resource::StencilMode::Write;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		psoDesc.FormatDS = formatDS;
		ZE_PSO_SET_NAME(psoDesc, "OutlineDraw");
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

		// Bind some scratch buffer
		data.Color.Bind(renderData.CL, ctx);
		renderData.Buffers.SetOutput(renderData.CL, ids.RenderTarget, ids.DepthStencil);
		renderData.CL.Draw(renderData.Dev, 0);

		renderData.CL.Close(renderData.Dev);
		renderData.Dev.ExecuteMain(renderData.CL);
	}
}