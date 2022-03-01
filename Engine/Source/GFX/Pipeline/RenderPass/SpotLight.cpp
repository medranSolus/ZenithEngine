#include "GFX/Pipeline/RenderPass/SpotLight.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::SpotLight
{
	Data* Setup(Device& dev, RendererBuildData& buildData, Info::World& worldData,
		PixelFormat formatColor, PixelFormat formatSpecular,
		PixelFormat formatShadow, PixelFormat formatShadowDepth)
	{
		Data* passData = new Data{ worldData };
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
		psoDesc.Blender = Resource::BlendType::Light;
		psoDesc.Culling = Resource::CullMode::Front;
		psoDesc.SetDepthClip(false);
		psoDesc.RenderTargetsCount = 2;
		psoDesc.FormatsRT[0] = formatColor;
		psoDesc.FormatsRT[1] = formatSpecular;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "SpotLight");
		passData->State.Init(dev, psoDesc, schema);

		passData->ShadowBuffers.emplace_back(dev, nullptr, static_cast<U32>(sizeof(TransformBuffer)), true);
		passData->TransformBuffers.emplace_back(dev, nullptr, static_cast<U32>(sizeof(TransformBuffer)), true);

		const auto volume = Primitive::MakeCone(8);
		passData->VolumeVB.Init(dev, { static_cast<U32>(volume.Vertices.size()), sizeof(Float3), volume.Vertices.data() });
		passData->VolumeIB.Init(dev, { static_cast<U32>(volume.Indices.size()), volume.Indices.data() });

		return passData;
	}

	void Execute(RendererExecuteData& renderData, PassData& passData)
	{
		Data& data = *reinterpret_cast<Data*>(passData.OptData);

		if (data.ShadowData.World.SpotLightInfo.Size)
		{
			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<Matrix, TransformBuffer, BUFFER_SHRINK_STEP>(renderData.Dev, data.ShadowBuffers, data.ShadowData.World.SpotLightInfo.Size);
			Utils::ResizeTransformBuffers<Matrix, TransformBuffer, BUFFER_SHRINK_STEP>(renderData.Dev, data.TransformBuffers, data.ShadowData.World.SpotLightInfo.Size);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			const auto& transforms = data.ShadowData.World.ActiveScene->TransformsGlobal;
			const auto& lights = data.ShadowData.World.ActiveScene->SpotLightBuffers;
			const auto& lightsData = data.ShadowData.World.ActiveScene->SpotLights;

			Resources ids = *passData.Buffers.CastConst<Resources>();
			for (U64 i = 0, j = 0; i < data.ShadowData.World.SpotLightInfo.Size; ++j)
			{
				auto& shadowCBuffer = data.ShadowBuffers.at(j);
				auto& cbuffer = data.TransformBuffers.at(j);
				TransformBuffer* shadowBuffer = reinterpret_cast<TransformBuffer*>(shadowCBuffer.GetRegion());
				TransformBuffer* buffer = reinterpret_cast<TransformBuffer*>(cbuffer.GetRegion());

				for (U32 k = 0; k < TransformBuffer::TRANSFORM_COUNT && i < data.ShadowData.World.SpotLightInfo.Size; ++k, ++i)
				{
					const auto& transform = transforms[data.ShadowData.World.SpotLights[i].TransformIndex];
					const auto& lightData = lightsData[i];
					const Matrix viewProjection = ShadowMap::Execute(renderData, data.ShadowData,
						*reinterpret_cast<ShadowMap::Resources*>(&ids.ShadowMap), transform.Position, lightData.Direction);

					const float volume = lights[i].Volume;
					Float3 translation = transform.Position;
					translation.y -= volume;
					const float circleScale = volume * tanf(lightData.OuterAngle + 0.22f);

					shadowBuffer->Transforms[k] = viewProjection;
					buffer->Transforms[k] = viewProjection *
						Math::XMMatrixTranspose(Math::XMMatrixScaling(circleScale, volume, circleScale) *
							Math::GetVectorRotation(Math::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f),
								Math::XMLoadFloat3(&lightData.Direction), true, volume) *
							Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&translation)));

					renderData.CL.Open(renderData.Dev, data.State);
					ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Spot Light nr_" + std::to_wstring(j)).c_str(), Pixel(0xFB, 0xE1, 0x06));
					renderData.Buffers.BarrierTransition(renderData.CL, ids.ShadowMap, Resource::State::RenderTarget, Resource::State::ShaderResourcePS);

					ctx.BindingSchema.SetGraphics(renderData.CL);
					renderData.Buffers.SetRTV<2>(renderData.CL, &ids.Color, true);

					Resource::Constant<U32> lightBatchId(renderData.Dev, k);
					lightBatchId.Bind(renderData.CL, ctx);
					Resource::Constant<Float3> lightPos(renderData.Dev, transform.Position);
					lightPos.Bind(renderData.CL, ctx);
					lights[i].Buffer.Bind(renderData.CL, ctx);

					shadowCBuffer.Bind(renderData.CL, ctx);
					cbuffer.Bind(renderData.CL, ctx);
					renderData.Buffers.SetSRV(renderData.CL, ctx, ids.ShadowMap);
					renderData.Buffers.SetSRV(renderData.CL, ctx, ids.GBufferNormal);
					data.ShadowData.World.DynamicDataBuffer.Bind(renderData.CL, ctx);
					renderData.EngineData.Bind(renderData.CL, ctx);
					ctx.Reset();

					data.VolumeVB.Bind(renderData.CL);
					data.VolumeIB.Bind(renderData.CL);

					renderData.CL.DrawIndexed(renderData.Dev, data.VolumeIB.GetCount());
					renderData.Buffers.BarrierTransition(renderData.CL, ids.ShadowMap, Resource::State::ShaderResourcePS, Resource::State::RenderTarget);
					ZE_DRAW_TAG_END(renderData.CL);
					renderData.CL.Close(renderData.Dev);
					renderData.Dev.ExecuteMain(renderData.CL);
				}
			}
		}
	}
}