#include "GFX/Pipeline/RenderPass/SpotLight.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::SpotLight
{
	static Expected<std::unique_ptr<PassExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) noexcept
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
		return desc;
	}

	Expected<std::unique_ptr<ExecuteData>> Initialize(Device& dev, RendererPassBuildData& buildData,
		PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth) noexcept
	{
		auto passData = std::make_unique<ExecuteData>();
		ShadowMap::Initialize(dev, buildData, passData->ShadowData, formatShadowDepth, formatShadow,
			Data::GetProjectionMatrix({ static_cast<float>(M_PI_2), 1.0f, 0.0001f }));

		Binding::SchemaDesc desc = {};
		desc.AddRange({ sizeof(Float3), 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light position
		desc.AddRange({ 1, 2, 5, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Spot light buffer
		desc.AddRange({ 1, 0, 6, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Shadow transform
		desc.AddRange({ 1, 0, 4, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Shadow map
		desc.AddRange({ 4, 1, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // GBuff
		desc.AddRange(buildData.DynamicDataRange, Resource::ShaderType::Pixel | Resource::ShaderType::Vertex);
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		ZE_EXPECT_RET_FAILED(passData->BindingIndex, buildData.BindingLib.AddDataBinding(dev, desc));

		const auto& schema = buildData.BindingLib.GetSchema(passData->BindingIndex);
		Resource::PipelineStateDesc psoDesc = {};
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.VS, "LightVS", buildData.ShaderCache));
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.PS, "SpotLightPS", buildData.ShaderCache));
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Blender = Resource::BlendType::Light;
		psoDesc.Culling = Resource::CullMode::Front;
		psoDesc.SetDepthClip(false);
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatLighting;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "SpotLight");
		ZE_EXPECT_RET_FAILED(passData->State, Resource::PipelineStateGfx::Create(dev, psoDesc, schema));

		const auto volume = Primitive::Cone::MakeSolid(8);
		Resource::MeshData meshData =
		{
			INVALID_EID, nullptr,
			ZE::Utils::SafeCast<U32>(volume.Vertices.size()),
			ZE::Utils::SafeCast<U32>(volume.Indices.size()),
			sizeof(Float3), 0
		};
		meshData.PackedMesh = Primitive::GetPackedMeshPackIndex(volume.Vertices, volume.Indices, meshData.IndexSize);
		ZE_EXPECT_RET_FAILED(passData->VolumeMesh, Resource::Mesh::Create(dev, buildData.Assets.GetDisk(), meshData));

		return passData;
	}

	Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		auto group = Data::GetSpotLightGroup();
		const U64 count = group.size();
		if (count)
		{
			ZE_PERF_GUARD("Spot Light - present");
			Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
			ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

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
				Matrix shadowMtx = {};
				ZE_EXPECT_RET_FAILED_CODE(shadowMtx, ShadowMap::Execute(dev, cl, renderData, data.ShadowData,
					*reinterpret_cast<ShadowMap::Resources*>(&ids.ShadowMap), transform.Position, lightData.Direction, lightFrustum));
				Math::XMStoreFloat4x4(&transformBuffer.TransformTps, shadowMtx);
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
				const float circleScale = light.Volume * std::tanf(lightData.OuterAngle + 0.22f);

				Resource::Constant<Float3> lightPos;
				ZE_EXPECT_RET_FAILED_CODE(lightPos, Resource::Constant<Float3>::Create(dev, transform.Position));
				lightPos.Bind(cl, ctx);
				light.Buffer.Bind(cl, ctx);
				ZE_CODE_RET_FAILED(cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(TransformBuffer)));

				Math::XMStoreFloat4x4(&transformBuffer.TransformTps, viewProjection *
					Math::XMMatrixTranspose(Math::XMMatrixScaling(circleScale, light.Volume, circleScale) *
						Math::GetVectorRotation({ 0.0f, -1.0f, 0.0f, 0.0f },
							Math::XMLoadFloat3(&lightData.Direction), true, light.Volume) *
						Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&translation))));

				ZE_CODE_RET_FAILED(cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(TransformBuffer)));
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
		return {};
	}
}