#include "GFX/Pipeline/RenderPass/Skybox.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::Skybox
{
	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, PassInitData* initData) noexcept
	{
		ZE_ASSERT(formats.size() == 2, "Incorrect size for Skybox initialization formats!");

		return Initialize(dev, buildData, formats.at(0), formats.at(1));
	}

	PassDesc GetDesc(PixelFormat formatRT, PixelFormat formatDS) noexcept
	{
		PassDesc desc{ Base(CorePassType::Skybox) };
		desc.InitializeFormats.reserve(2);
		desc.InitializeFormats.emplace_back(formatRT);
		desc.InitializeFormats.emplace_back(formatDS);
		desc.Init = Initialize;
		desc.Execute = Execute;
		return desc;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();

		Binding::SchemaDesc desc = {};
		desc.AddRange({ 1, 0, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Skybox
		desc.AddRange(buildData.DynamicDataRange, Resource::ShaderType::Vertex);
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		ZE_EXPECT_RET_FAILED(passData->BindingIndex, buildData.BindingLib.AddDataBinding(dev, desc));

		const std::vector<Float3> vertices = Primitive::Cube::MakeSolidVertex();
		const std::vector<U16> indices = Primitive::Cube::MakeSolidIndexInverted();
		Resource::MeshData meshData =
		{
			INVALID_EID, nullptr,
			Utils::SafeCast<U32>(vertices.size()),
			Utils::SafeCast<U32>(indices.size()),
			sizeof(Float3), 0
		};
		meshData.PackedMesh = Primitive::GetPackedMeshPackIndex(vertices, indices, meshData.IndexSize);
		ZE_EXPECT_RET_FAILED(passData->MeshData, Resource::Mesh::Create(dev, buildData.Assets.GetDisk(), meshData));

		Resource::PipelineStateDesc psoDesc = {};
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.VS, "SkyboxVS", buildData.ShaderCache));
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.PS, "SkyboxPS", buildData.ShaderCache));
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthBefore;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "Skybox");
		ZE_EXPECT_RET_FAILED(passData->State, Resource::PipelineStateGfx::Create(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex)));

		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		ZE_PERF_GUARD("Skybox");
		Resources ids = *reinterpret_cast<const Resources*>(passData.Resources.get());
		ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

		ZE_DRAW_TAG_BEGIN(dev, cl, "Skybox", Pixel(0x82, 0xCA, 0xFA));
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget, ids.DepthStencil);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.Skybox);
		renderData.BindRendererDynamicData(cl, ctx);
		renderData.SettingsBuffer.Bind(cl, ctx);
		data.MeshData.Draw(dev, cl);

		renderData.Buffers.EndRaster(cl);
		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}
}