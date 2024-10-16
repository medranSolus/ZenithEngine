#include "GFX/Pipeline/RenderPass/SpotLight.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::SpotLight
{
	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->State.Free(dev);
		execData->VolumeMesh.Free(dev);
		ShadowMap::Clean(dev, execData->ShadowData);
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData,
		PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth)
	{
		ExecuteData* passData = new ExecuteData;
		ShadowMap::Setup(dev, buildData, passData->ShadowData, formatShadowDepth, formatShadow,
			Data::GetProjectionMatrix({ static_cast<float>(M_PI_2), 1.0f, 0.0001f }));

		if (buildData.BindingLib.FetchBinding("light", passData->BindingIndex))
		{
			Binding::SchemaDesc desc;
			desc.AddRange({ sizeof(Float3), 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light position
			desc.AddRange({ 1, 2, 5, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Spot light buffer
			desc.AddRange({ 1, 0, 6, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Shadow transform
			desc.AddRange({ 1, 0, 4, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
			desc.AddRange({ 1, 0, 3, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Shadow map
			desc.AddRange({ 4, 1, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // GBuff
			desc.AddRange({ 1, 12, 1, Resource::ShaderType::Vertex | Resource::ShaderType::Pixel, Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer }); // Renderer dynamic data
			desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
			passData->BindingIndex = buildData.BindingLib.RegisterCommonBinding(dev, desc, "light");
		}

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

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		auto group = Data::GetSpotLightGroup();
		const U64 count = group.size();
		if (count)
		{
			ZE_PERF_GUARD("Spot Light - present");
			const RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
			const CameraPBR& dynamicData = *reinterpret_cast<CameraPBR*>(renderData.DynamicData);
			const Matrix viewProjection = Math::XMLoadFloat4x4(&dynamicData.ViewProjectionTps);
			const Matrix lightProjections = Math::XMLoadFloat4x4(&data.ShadowData.Projection);
			const Vector cameraPos = Math::XMLoadFloat3(&dynamicData.CameraPos);

			Math::BoundingFrustum frustum = Data::GetFrustum(Math::XMLoadFloat4x4(&renderer.GetProjection()), Settings::MaxRenderDistance);
			frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&renderer.GetCameraRotation()), cameraPos);

			Resources ids = *passData.Buffers.CastConst<Resources>();
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			auto& cbuffer = *renderData.DynamicBuffer;
			cl.Open(dev);
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
				TransformBuffer transformBuffer;
				Math::XMStoreFloat4x4(&transformBuffer.TransformTps, ShadowMap::Execute(dev, cl, renderData, data.ShadowData,
					*reinterpret_cast<ShadowMap::Resources*>(&ids.ShadowMap),
					transform.Position, lightData.Direction, lightFrustum));
				ZE_PERF_STOP();

				ZE_PERF_START("Spot Light - after shadow map");
				data.State.Bind(cl);
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Spot Light nr_" + std::to_string(i)).c_str(), Pixel(0xFB, 0xE1, 0x06));
				renderData.Buffers.BarrierTransition(cl, ids.ShadowMap, Resource::StateRenderTarget, Resource::StateShaderResourcePS);

				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetRTV(cl, ids.Lighting);

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
				ctx.Reset();

				data.VolumeMesh.Draw(dev, cl);
				renderData.Buffers.BarrierTransition(cl, ids.ShadowMap, Resource::StateShaderResourcePS, Resource::StateRenderTarget);
				ZE_DRAW_TAG_END(dev, cl);
				ZE_PERF_STOP();
			}
			ZE_PERF_STOP();

			cl.Close(dev);
			dev.ExecuteMain(cl);
		}
	}
}