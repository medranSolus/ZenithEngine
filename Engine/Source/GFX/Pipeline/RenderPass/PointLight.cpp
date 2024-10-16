#include "GFX/Pipeline/RenderPass/PointLight.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::PointLight
{
	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->State.Free(dev);
		execData->VolumeMesh.Free(dev);
		ShadowMapCube::Clean(dev, execData->ShadowData);
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData,
		PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth)
	{
		ExecuteData* passData = new ExecuteData;
		ShadowMapCube::Setup(dev, buildData, passData->ShadowData, formatShadowDepth, formatShadow);

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(Float3), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light position
		desc.AddRange({ 1, 1, 5, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Point light buffer
		desc.AddRange({ 1, 0, 4, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Shadow map
		desc.AddRange({ 4, 1, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // GBuff normal, specular, depth
		desc.AddRange({ 1, 12, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer }); // Renderer dynamic data
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
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

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		auto group = Data::GetPointLightGroup();
		const U64 count = group.size();
		if (count)
		{
			ZE_PERF_GUARD("Point Light - present");
			const RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
			const CameraPBR& dynamicData = *reinterpret_cast<CameraPBR*>(renderData.DynamicData);
			const Matrix viewProjection = Math::XMLoadFloat4x4(&dynamicData.ViewProjectionTps);
			const Vector cameraPos = Math::XMLoadFloat3(&dynamicData.CameraPos);

			Math::BoundingFrustum frustum = Data::GetFrustum(Math::XMLoadFloat4x4(&renderer.GetProjection()), Settings::MaxRenderDistance);
			frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&renderer.GetCameraRotation()), cameraPos);

			Resources ids = *passData.Buffers.CastConst<Resources>();
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			auto& cbuffer = *renderData.DynamicBuffer;
			cl.Open(dev);
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
				ShadowMapCube::Execute(dev, cl, renderData, data.ShadowData,
					*reinterpret_cast<ShadowMapCube::Resources*>(&ids.ShadowMap), transform.Position, light.Volume);
				ZE_PERF_STOP();

				ZE_PERF_START("Point Light - after shadow map");
				TransformBuffer transformBuffer;
				Math::XMStoreFloat4x4(&transformBuffer.TransformTps, viewProjection *
					Math::XMMatrixTranspose(Math::XMMatrixScaling(light.Volume, light.Volume, light.Volume) *
						Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&transform.Position))));

				data.State.Bind(cl);
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Point Light nr_" + std::to_string(i)).c_str(), Pixel(0xFD, 0xFB, 0xD3));
				renderData.Buffers.BarrierTransition(cl, ids.ShadowMap, Resource::StateRenderTarget, Resource::StateShaderResourcePS);

				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetRTV(cl, ids.Lighting);

				Resource::Constant<Float3> lightPos(dev, transform.Position);
				lightPos.Bind(cl, ctx);
				light.Buffer.Bind(cl, ctx);

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