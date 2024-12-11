#include "GFX/Pipeline/RenderPass/PointLight.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::PointLight
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 3, "Incorrect size for PointLight initialization formats!");
		return Initialize(dev, buildData, formats.at(0), formats.at(1), formats.at(2));
	}

	PassDesc GetDesc(PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth) noexcept
	{
		PassDesc desc{ Base(CorePassType::PointLight) };
		desc.InitializeFormats.reserve(3);
		desc.InitializeFormats.emplace_back(formatLighting);
		desc.InitializeFormats.emplace_back(formatShadow);
		desc.InitializeFormats.emplace_back(formatShadowDepth);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Clean = Clean;
		return desc;
	}

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		syncStatus.SyncMain(dev);
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->State.Free(dev);
		execData->VolumeMesh.Free(dev);
		ShadowMapCube::Clean(dev, execData->ShadowData);
		delete execData;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData,
		PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth)
	{
		ExecuteData* passData = new ExecuteData;
		ShadowMapCube::Initialize(dev, buildData, passData->ShadowData, formatShadowDepth, formatShadow);

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(Float3), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light position
		desc.AddRange({ 1, 1, 5, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Point light buffer
		desc.AddRange({ 1, 0, 4, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Shadow map
		desc.AddRange({ 4, 1, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // GBuff normal, specular, depth
		desc.AddRange(buildData.DynamicDataRange, Resource::ShaderType::Pixel);
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		const auto& schema = buildData.BindingLib.GetSchema(passData->BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "LightVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, "PointLightPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Blender = Resource::BlendType::Light;
		psoDesc.Culling = Resource::CullMode::Front;
		psoDesc.SetDepthClip(false);
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatLighting;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "PointLight");
		passData->State.Init(dev, psoDesc, schema);

		const auto volume = Primitive::MakeSphereIcoSolid(3);
		passData->VolumeMesh.Init(dev, buildData.Assets.GetDisk(),
			{
				INVALID_EID, Primitive::GetPackedMesh(volume.Vertices, volume.Indices),
				ZE::Utils::SafeCast<U32>(volume.Vertices.size()),
				ZE::Utils::SafeCast<U32>(volume.Indices.size()),
				sizeof(Float3), sizeof(U32)
			});
		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		auto group = Data::GetPointLightGroup();
		const U64 count = group.size();
		if (count)
		{
			ZE_PERF_GUARD("Point Light - present");
			Resources ids = *passData.Resources.CastConst<Resources>();
			ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

			const Matrix viewProjection = Math::XMLoadFloat4x4(&renderData.DynamicData.ViewProjectionTps);
			const Vector cameraPos = Math::XMLoadFloat3(&renderData.DynamicData.CameraPos);

			Math::BoundingFrustum frustum = Data::GetFrustum(Math::XMLoadFloat4x4(&renderData.GraphData.Projection), Settings::MaxRenderDistance);
			frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&Settings::Data.get<Data::TransformGlobal>(renderData.GraphData.CurrentCamera).Rotation), cameraPos);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			auto& cbuffer = *renderData.DynamicBuffer;
			ZE_PERF_START("Point Light - main loop");
			for (U64 i = 0; i < count; ++i)
			{
				ZE_PERF_GUARD("Point Light - single loop item");
				EID entity = group[i];
				const auto& transform = group.get<Data::TransformGlobal>(entity);
				const auto& light = group.get<Data::PointLightBuffer>(entity);

				// Check if light will be visible in current view
				const Math::BoundingSphere lightSphere(transform.Position, light.Volume);
				if (!frustum.Intersects(lightSphere))
					continue;

				ZE_PERF_START("Point Light - shadow map");
				ShadowMapCube::Execute(dev, cl, renderData, data.ShadowData, *reinterpret_cast<ShadowMapCube::Resources*>(&ids.ShadowMap), transform.Position, light.Volume);
				ZE_PERF_STOP();

				ZE_PERF_START("Point Light - after shadow map");
				TransformBuffer transformBuffer = {};
				Math::XMStoreFloat4x4(&transformBuffer.TransformTps, viewProjection *
					Math::XMMatrixTranspose(Math::XMMatrixScaling(light.Volume, light.Volume, light.Volume) *
						Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&transform.Position))));

				ZE_DRAW_TAG_BEGIN(dev, cl, ("Point Light nr_" + std::to_string(i)).c_str(), Pixel(0xFD, 0xFB, 0xD3));
				renderData.Buffers.BeginRaster(cl, ids.Lighting);
				renderData.Buffers.Barrier(cl, BarrierTransition{ ids.ShadowMap, TextureLayout::RenderTarget, TextureLayout::ShaderResource,
					Base(ResourceAccess::RenderTarget), Base(ResourceAccess::ShaderResource), Base(StageSync::RenderTarget), Base(StageSync::PixelShading) });

				ctx.Reset();
				ctx.BindingSchema.SetGraphics(cl);
				data.State.Bind(cl);

				Resource::Constant<Float3> lightPos(dev, transform.Position);
				lightPos.Bind(cl, ctx);
				light.Buffer.Bind(cl, ctx);

				cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(TransformBuffer));
				renderData.Buffers.SetSRV(cl, ctx, ids.ShadowMap);
				renderData.Buffers.SetSRV(cl, ctx, ids.GBufferDepth);
				renderData.BindRendererDynamicData(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				data.VolumeMesh.Draw(dev, cl);

				renderData.Buffers.Barrier(cl, BarrierTransition{ ids.ShadowMap, TextureLayout::ShaderResource, TextureLayout::RenderTarget,
					Base(ResourceAccess::ShaderResource), Base(ResourceAccess::RenderTarget), Base(StageSync::PixelShading), Base(StageSync::RenderTarget) });
				renderData.Buffers.EndRaster(cl);
				ZE_DRAW_TAG_END(dev, cl);
				ZE_PERF_STOP();
			}
			ZE_PERF_STOP();
		}
	}
}