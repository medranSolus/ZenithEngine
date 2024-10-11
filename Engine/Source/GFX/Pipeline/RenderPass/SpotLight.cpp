#include "GFX/Pipeline/RenderPass/SpotLight.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::SpotLight
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 3, "Incorrect size for SpotLight initialization formats!");
		return Initialize(dev, buildData, formats.at(0), formats.at(1), formats.at(2));
	}

	PassDesc GetDesc(PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth) noexcept
	{
		PassDesc desc{ Base(CorePassType::SpotLight) };
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

	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->State.Free(dev);
		execData->VolumeMesh.Free(dev);
		ShadowMap::Clean(dev, execData->ShadowData);
		delete execData;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData,
		PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth)
	{
		ExecuteData* passData = new ExecuteData;
		ShadowMap::Initialize(dev, buildData, passData->ShadowData, formatShadowDepth, formatShadow,
			Data::GetProjectionMatrix({ static_cast<float>(M_PI_2), 1.0f, 0.0001f }));

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(Float3), 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light position
		desc.AddRange({ 1, 2, 5, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Spot light buffer
		desc.AddRange({ 1, 0, 6, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Shadow transform
		desc.AddRange({ 1, 0, 4, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Shadow map
		desc.AddRange({ 4, 1, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // GBuff
		desc.AddRange(buildData.DynamicDataRange, Resource::ShaderType::Pixel | Resource::ShaderType::Vertex);
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		const auto& schema = buildData.BindingLib.GetSchema(passData->BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "LightVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, "SpotLightPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Blender = Resource::BlendType::Light;
		psoDesc.Culling = Resource::CullMode::Front;
		psoDesc.SetDepthClip(false);
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatLighting;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "SpotLight");
		passData->State.Init(dev, psoDesc, schema);

		const auto volume = Primitive::MakeConeSolid(8);
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
		auto group = Data::GetSpotLightGroup();
		const U64 count = group.size();
		if (count)
		{
			ZE_PERF_GUARD("Spot Light - present");
			Resources ids = *passData.Resources.CastConst<Resources>();
			ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

			const Matrix viewProjection = Math::XMLoadFloat4x4(&renderData.DynamicData.ViewProjectionTps);
			const Matrix lightProjections = Math::XMLoadFloat4x4(&data.ShadowData.Projection);
			const Vector cameraPos = Math::XMLoadFloat3(&renderData.DynamicData.CameraPos);

			Math::BoundingFrustum frustum = Data::GetFrustum(Math::XMLoadFloat4x4(&renderData.GraphData.Projection), Settings::MaxRenderDistance);
			frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&Settings::Data.get<Data::TransformGlobal>(renderData.GraphData.CurrentCamera).Rotation), cameraPos);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			auto& cbuffer = *renderData.DynamicBuffer;
			ZE_PERF_START("Spot Light - main loop");
			for (U64 i = 0; i < count; ++i)
			{
				ZE_PERF_GUARD("Spot Light - single loop item");
				EID entity = group[i];
				const auto& transform = group.get<Data::TransformGlobal>(entity);
				const auto& lightData = group.get<Data::SpotLight>(entity);
				const auto& light = group.get<Data::SpotLightBuffer>(entity);

				// Check if light will be visible in current view
				Math::BoundingFrustum lightFrustum = Data::GetFrustum(lightProjections, light.Volume);
				lightFrustum.Transform(lightFrustum, 1.0f,
					Math::XMQuaternionRotationMatrix(Math::GetVectorRotation({ 0.0f, 0.0f, 1.0f, 0.0f },
						Math::XMLoadFloat3(&lightData.Direction))), Math::XMLoadFloat3(&transform.Position));
				if (!frustum.Intersects(lightFrustum))
					continue;

				ZE_PERF_START("Spot Light - shadow map");
				TransformBuffer transformBuffer = {};
				Math::XMStoreFloat4x4(&transformBuffer.TransformTps, ShadowMap::Execute(dev, cl, renderData, data.ShadowData,
					*reinterpret_cast<ShadowMap::Resources*>(&ids.ShadowMap), transform.Position, lightData.Direction, lightFrustum));
				ZE_PERF_STOP();

				ZE_PERF_START("Spot Light - after shadow map");
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Spot Light nr_" + std::to_string(i)).c_str(), Pixel(0xFB, 0xE1, 0x06));
				renderData.Buffers.BeginRaster(cl, ids.Lighting);
				renderData.Buffers.Barrier(cl, BarrierTransition{ ids.ShadowMap, TextureLayout::RenderTarget, TextureLayout::ShaderResource,
					Base(ResourceAccess::RenderTarget), Base(ResourceAccess::ShaderResource), Base(StageSync::RenderTarget), Base(StageSync::PixelShading) });

				ctx.Reset();
				ctx.BindingSchema.SetGraphics(cl);
				data.State.Bind(cl);

				Float3 translation = transform.Position;
				translation.y -= light.Volume;
				const float circleScale = light.Volume * tanf(lightData.OuterAngle + 0.22f);

				Resource::Constant<Float3> lightPos(dev, transform.Position);
				lightPos.Bind(cl, ctx);
				light.Buffer.Bind(cl, ctx);
				cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(TransformBuffer));

				Math::XMStoreFloat4x4(&transformBuffer.TransformTps, viewProjection *
					Math::XMMatrixTranspose(Math::XMMatrixScaling(circleScale, light.Volume, circleScale) *
						Math::GetVectorRotation({ 0.0f, -1.0f, 0.0f, 0.0f },
							Math::XMLoadFloat3(&lightData.Direction), true, light.Volume) *
						Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&translation))));

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