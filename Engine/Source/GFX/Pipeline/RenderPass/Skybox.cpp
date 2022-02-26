#include "GFX/Pipeline/RenderPass/Skybox.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::Skybox
{
	Data* Setup(Device& dev, RendererBuildData& buildData, Resource::CBuffer& worldDataBuffer,
		PixelFormat formatRT, PixelFormat formatDS, const std::string& cubemapPath, const std::string& cubemapExt)
	{
		Data* passData = new Data{ worldDataBuffer };

		// Create skybox Cube/Dome and use it's vertex and index buffers

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 1, 12, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Texture::PackDesc texDesc;
		texDesc.Options = Resource::Texture::PackOption::StaticCreation;
		std::vector<Surface> textures;
		textures.reserve(6);
		textures.emplace_back(cubemapPath + "/px" + cubemapExt); // Right
		textures.emplace_back(cubemapPath + "/nx" + cubemapExt); // Left
		textures.emplace_back(cubemapPath + "/py" + cubemapExt); // Up
		textures.emplace_back(cubemapPath + "/ny" + cubemapExt); // Down
		textures.emplace_back(cubemapPath + "/pz" + cubemapExt); // Front
		textures.emplace_back(cubemapPath + "/nz" + cubemapExt); // Back
		texDesc.AddTexture(Resource::Texture::Type::Cube, Resource::Texture::Usage::PixelShader, std::move(textures));
		passData->SkyTexture.Init(dev, texDesc);

		const std::vector<Float3> vertices = Primitive::MakeCubeVertex();
		passData->VertexBuffer.Init(dev, { static_cast<U32>(vertices.size()), sizeof(Float3), vertices.data() });
		const std::vector<U32> indices = Primitive::MakeCubeIndexInverted();
		passData->IndexBuffer.Init(dev, { static_cast<U32>(indices.size()), indices.data() });
		dev.StartUpload();

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"SkyboxVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"SkyboxPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthBefore;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "Skybox");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	void Execute(RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		Data& data = *reinterpret_cast<Data*>(passData.OptData);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		renderData.CL.Open(renderData.Dev, data.State);
		ZE_DRAW_TAG_BEGIN(renderData.CL, L"Skybox", Pixel(0x82, 0xCA, 0xFA));
		ctx.BindingSchema.SetGraphics(renderData.CL);

		renderData.Buffers.SetOutput(renderData.CL, ids.RenderTarget, ids.DepthStencil);
		data.SkyTexture.Bind(renderData.CL, ctx);
		data.WorldDataBuffer.Bind(renderData.CL, ctx);
		renderData.EngineData.Bind(renderData.CL, ctx);
		data.VertexBuffer.Bind(renderData.CL);
		data.IndexBuffer.Bind(renderData.CL);
		renderData.CL.DrawIndexed(renderData.Dev, data.IndexBuffer.GetCount());

		ZE_DRAW_TAG_END(renderData.CL);
		renderData.CL.Close(renderData.Dev);
		renderData.Dev.ExecuteMain(renderData.CL);
	}
}