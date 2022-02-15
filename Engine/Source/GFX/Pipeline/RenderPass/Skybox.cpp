#include "GFX/Pipeline/RenderPass/Skybox.h"

namespace ZE::GFX::Pipeline::RenderPass::Skybox
{
	Data* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS,
		const std::string& cubemapPath, const std::string& cubemapExt)
	{
		Data* passData = new Data;

		// Create skybox Cube/Dome and use it's vertex and index buffers

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Vertex | Resource::ShaderType::Pixel);
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

		constexpr float POINT = 0.5f;
		Float3 vertices[]
		{
			{ -POINT, -POINT, -POINT },
			{ -POINT, POINT, -POINT },
			{ POINT, POINT, -POINT },
			{ POINT, -POINT, -POINT },
			{ -POINT, -POINT, POINT },
			{ -POINT, POINT, POINT },
			{ POINT, POINT, POINT },
			{ POINT, -POINT, POINT }
		};
		passData->VertexBuffer.Init(dev, { sizeof(vertices) / sizeof(Float3), sizeof(Float3), vertices });
		const U32 indices[]
		{
			2,1,0, 3,2,0, // Front
			1,5,4, 0,1,4, // Left
			5,6,7, 4,5,7, // Back
			6,2,3, 7,6,3, // Right
			6,5,1, 2,6,1, // Top
			3,0,4, 7,3,4  // Down
		};
		passData->IndexBuffer.Init(dev, { sizeof(indices) / sizeof(U32), indices });
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
		renderData.EngineData.Bind(renderData.CL, ctx);
		data.VertexBuffer.Bind(renderData.CL);
		data.IndexBuffer.Bind(renderData.CL);
		renderData.CL.DrawIndexed(renderData.Dev, data.IndexBuffer.GetCount());

		ZE_DRAW_TAG_END(renderData.CL);
		renderData.CL.Close(renderData.Dev);
		renderData.Dev.ExecuteMain(renderData.CL);
	}
}