#include "GFX/Pipeline/RenderPass/Skybox.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::Skybox
{
	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->State.Free(dev);
		execData->SkyTexture.Free(dev);
		execData->MeshData.Free(dev);
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT,
		PixelFormat formatDS, const std::string& cubemapPath, const std::string& cubemapExt)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Skybox
		desc.AddRange({ 1, 12, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer }); // Renderer dynamic data
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Texture::PackDesc texDesc;
		texDesc.Options = Resource::Texture::PackOption::StaticCreation;
		std::vector<Surface> textures;
		textures.reserve(6);
		bool result = true;
		result &= textures.emplace_back().Load(cubemapPath + "/px" + cubemapExt); // Right
		result &= textures.emplace_back().Load(cubemapPath + "/nx" + cubemapExt); // Left
		result &= textures.emplace_back().Load(cubemapPath + "/py" + cubemapExt); // Up
		result &= textures.emplace_back().Load(cubemapPath + "/ny" + cubemapExt); // Down
		result &= textures.emplace_back().Load(cubemapPath + "/pz" + cubemapExt); // Front
		result &= textures.emplace_back().Load(cubemapPath + "/nz" + cubemapExt); // Back
		if (!result)
			throw ZE_RGC_EXCEPT("Error loading cubemap!");
		texDesc.AddTexture(Resource::Texture::Type::Cube, Resource::Texture::Usage::PixelShader, std::move(textures));
		passData->SkyTexture.Init(dev, buildData.Assets.GetDisk(), texDesc);

		const std::vector<Float3> vertices = Primitive::MakeCubeSolidVertex();
		const std::vector<U32> indices = Primitive::MakeCubeSolidIndexInverted();
		passData->MeshData.Init(dev, buildData.Assets.GetDisk(),
			{
				INVALID_EID,
				vertices.data(), indices.data(),
				Utils::SafeCast<U32>(vertices.size()),
				Utils::SafeCast<U32>(indices.size()),
				sizeof(Float3), sizeof(U32)
			});

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "SkyboxVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, "SkyboxPS", buildData.ShaderCache);
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

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Skybox");
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		cl.Open(dev, data.State);
		ZE_DRAW_TAG_BEGIN(dev, cl, "Skybox", Pixel(0x82, 0xCA, 0xFA));
		ctx.BindingSchema.SetGraphics(cl);

		renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.DepthStencil);
		data.SkyTexture.Bind(cl, ctx);
		renderData.BindRendererDynamicData(cl, ctx);
		renderData.SettingsBuffer.Bind(cl, ctx);
		data.MeshData.Draw(dev, cl);

		ZE_DRAW_TAG_END(dev, cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}