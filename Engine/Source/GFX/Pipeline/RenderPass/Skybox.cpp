#include "GFX/Pipeline/RenderPass/Skybox.h"

namespace ZE::GFX::Pipeline::RenderPass::Skybox
{
	Data* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS)
	{
		Data* passData = new Data;

		// Find a way to bind camera to the pass (or maybe use the one supplied by renderer?
		// Create skybox Cube/Dome and use it's vertex and index buffers

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Vertex | Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Texture::PackDesc texDesc;
		std::vector<Surface> textures;
		texDesc.AddTexture(Resource::Texture::Type::Cube, Resource::Texture::Usage::PixelShader, std::move(textures));
		passData->SkyTexture.Init(dev, texDesc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"SkyboxVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"SkyboxPS", buildData.ShaderCache);
		psoDesc.Stencil = Resource::StencilMode::DepthFirst;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		psoDesc.FormatDS = formatDS;
		ZE_PSO_SET_NAME(psoDesc, "Skybox");
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

		// Also bind temporary or not CBV
		data.SkyTexture.Bind(renderData.CL, ctx);
		renderData.EngineData.Bind(renderData.CL, ctx);
		renderData.Buffers.SetOutput(renderData.CL, ids.RenderTarget, ids.DepthStencil);
		renderData.CL.Draw(renderData.Dev, 0);

		renderData.CL.Close(renderData.Dev);
		renderData.Dev.ExecuteMain(renderData.CL);
	}
}