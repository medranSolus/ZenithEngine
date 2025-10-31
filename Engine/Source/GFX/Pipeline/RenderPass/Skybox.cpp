#include "GFX/Pipeline/RenderPass/Skybox.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::Skybox
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
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
		desc.Clean = Clean;
		return desc;
	}

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		syncStatus.SyncMain(dev);
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->State.Free(dev);
		execData->MeshData.Free(dev);
		delete execData;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Skybox
		desc.AddRange(buildData.DynamicDataRange, Resource::ShaderType::Vertex);
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		const std::vector<Float3> vertices = Primitive::MakeCubeSolidVertex();
		const std::vector<U32> indices = Primitive::MakeCubeSolidIndexInverted();
		passData->MeshData.Init(dev, buildData.Assets.GetDisk(),
			{
				INVALID_EID, Primitive::GetPackedMesh(vertices, indices),
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

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Skybox");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

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