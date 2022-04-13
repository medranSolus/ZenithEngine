#include "GFX/Pipeline/RenderPass/SpotLight.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Pipeline/DataPBR.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::SpotLight
{
	void Clean(void* data)
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		ShadowMap::Clean(execData->ShadowData);
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData,
		PixelFormat formatColor, PixelFormat formatSpecular,
		PixelFormat formatShadow, PixelFormat formatShadowDepth)
	{
		ExecuteData* passData = new ExecuteData;
		ShadowMap::Setup(dev, buildData, passData->ShadowData, formatShadowDepth, formatShadow,
			Math::XMMatrixPerspectiveFovLH(static_cast<float>(M_PI_2), 1.0f, 0.01f, 1000.0f));

		if (buildData.BindingLib.FetchBinding("light", passData->BindingIndex))
		{
			Binding::SchemaDesc desc;
			desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Vertex | Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
			desc.AddRange({ sizeof(Float3), 2, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
			desc.AddRange({ 1, 3, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
			desc.AddRange({ 1, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
			desc.AddRange({ 1, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
			desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
			desc.AddRange({ 3, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
			desc.AddRange({ 1, 12, Resource::ShaderType::Vertex | Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
			desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
			passData->BindingIndex = buildData.BindingLib.RegisterCommonBinding(dev, desc, "light");
		}

		const auto& schema = buildData.BindingLib.GetSchema(passData->BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"LightVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"SpotLightPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Blender = Resource::BlendType::Light;
		psoDesc.Culling = Resource::CullMode::Front;
		psoDesc.SetDepthClip(false);
		psoDesc.RenderTargetsCount = 2;
		psoDesc.FormatsRT[0] = formatColor;
		psoDesc.FormatsRT[1] = formatSpecular;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "SpotLight");
		passData->State.Init(dev, psoDesc, schema);

		passData->ShadowBuffers.Exec([&dev](auto& x) { x.emplace_back(dev, nullptr, static_cast<U32>(sizeof(TransformBuffer)), true); });
		passData->TransformBuffers.Exec([&dev](auto& x) { x.emplace_back(dev, nullptr, static_cast<U32>(sizeof(TransformBuffer)), true); });

		const auto volume = Primitive::MakeConeSolid(8);
		passData->VolumeVB.Init(dev, { static_cast<U32>(volume.Vertices.size()), sizeof(Float3), volume.Vertices.data() });
		passData->VolumeIB.Init(dev, { static_cast<U32>(volume.Indices.size()), volume.Indices.data() });

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);
		data.ShadowData.PreviousEntityCount = 0;

		auto group = Data::GetSpotLightGroup(renderData.Registry);
		const U64 count = group.size();
		if (count)
		{
			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<Matrix, TransformBuffer, BUFFER_SHRINK_STEP>(dev, data.ShadowBuffers.Get(), count);
			Utils::ResizeTransformBuffers<Matrix, TransformBuffer, BUFFER_SHRINK_STEP>(dev, data.TransformBuffers.Get(), count);

			const Matrix viewProjection = reinterpret_cast<CameraPBR*>(renderData.DynamicData)->ViewProjection;
			Resources ids = *passData.Buffers.CastConst<Resources>();
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			for (U64 i = 0, j = 0; i < count; ++j)
			{
				auto& shadowCBuffer = data.ShadowBuffers.Get().at(j);
				auto& cbuffer = data.TransformBuffers.Get().at(j);

				for (U32 k = 0; k < TransformBuffer::TRANSFORM_COUNT && i < count; ++k, ++i)
				{
					TransformBuffer* shadowBuffer = reinterpret_cast<TransformBuffer*>(shadowCBuffer.GetRegion(dev));
					TransformBuffer* buffer = reinterpret_cast<TransformBuffer*>(cbuffer.GetRegion(dev));

					auto entity = group[i];
					const auto& transform = group.get<Data::TransformGlobal>(entity);
					const auto& lightData = group.get<Data::SpotLight>(entity);
					shadowBuffer->Transforms[k] = ShadowMap::Execute(dev, cl, renderData, data.ShadowData,
						*reinterpret_cast<ShadowMap::Resources*>(&ids.ShadowMap), transform.Position, lightData.Direction);

					const auto& light = group.get<Data::SpotLightBuffer>(entity);
					Float3 translation = transform.Position;
					translation.y -= light.Volume;
					const float circleScale = light.Volume * tanf(lightData.OuterAngle + 0.22f);

					buffer->Transforms[k] = viewProjection *
						Math::XMMatrixTranspose(Math::XMMatrixScaling(circleScale, light.Volume, circleScale) *
							Math::GetVectorRotation({ 0.0f, -1.0f, 0.0f, 0.0f },
								Math::XMLoadFloat3(&lightData.Direction), true, light.Volume) *
							Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&translation)));

					cl.Open(dev, data.State);
					ZE_DRAW_TAG_BEGIN(cl, (L"Spot Light nr_" + std::to_wstring(i)).c_str(), Pixel(0xFB, 0xE1, 0x06));
					renderData.Buffers.BarrierTransition(cl, ids.ShadowMap, Resource::State::RenderTarget, Resource::State::ShaderResourcePS);

					ctx.BindingSchema.SetGraphics(cl);
					renderData.Buffers.SetRTV<2>(cl, &ids.Color, true);

					Resource::Constant<U32> lightBatchId(dev, k);
					lightBatchId.Bind(cl, ctx);
					Resource::Constant<Float3> lightPos(dev, transform.Position);
					lightPos.Bind(cl, ctx);
					light.Buffer.Bind(cl, ctx);

					shadowCBuffer.Bind(cl, ctx);
					cbuffer.Bind(cl, ctx);
					renderData.Buffers.SetSRV(cl, ctx, ids.ShadowMap);
					renderData.Buffers.SetSRV(cl, ctx, ids.GBufferNormal);
					renderData.DynamicBuffers.Get().Bind(cl, ctx);
					renderData.SettingsBuffer.Bind(cl, ctx);
					ctx.Reset();

					data.VolumeVB.Bind(cl);
					data.VolumeIB.Bind(cl);

					cbuffer.FlushRegion(dev);
					cl.DrawIndexed(dev, data.VolumeIB.GetCount());
					renderData.Buffers.BarrierTransition(cl, ids.ShadowMap, Resource::State::ShaderResourcePS, Resource::State::RenderTarget);
					ZE_DRAW_TAG_END(cl);
					cl.Close(dev);
					dev.ExecuteMain(cl);
				}
			}
		}
	}
}