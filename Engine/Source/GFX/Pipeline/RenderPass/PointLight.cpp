#include "GFX/Pipeline/RenderPass/PointLight.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::PointLight
{
	Data* Setup(Device& dev, RendererBuildData& buildData, Info::World& worldData,
		PixelFormat formatColor, PixelFormat formatSpecular,
		PixelFormat formatShadow, PixelFormat formatShadowDepth)
	{
		Data* passData = new Data{ worldData };
		ShadowMapCube::Setup(dev, buildData, passData->ShadowData, formatShadowDepth, formatShadow);

		if (buildData.BindingLib.FetchBinding("light", passData->BindingIndex))
		{
			Binding::SchemaDesc desc;
			desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Vertex, Binding::RangeFlag::Constant });
			desc.AddRange({ sizeof(Float3), 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
			desc.AddRange({ 1, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
			desc.AddRange({ 1, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
			desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
			desc.AddRange({ 3, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
			desc.AddRange({ 1, 12, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
			desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
			passData->BindingIndex = buildData.BindingLib.RegisterCommonBinding(dev, desc, "light");
		}

		const auto& schema = buildData.BindingLib.GetSchema(passData->BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"LightVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"PointLightPS", buildData.ShaderCache);
		psoDesc.Blender = Resource::BlendType::Light;
		psoDesc.Culling = Resource::CullMode::Front;
		psoDesc.SetDepthClip(false);
		psoDesc.RenderTargetsCount = 2;
		psoDesc.FormatsRT[0] = formatColor;
		psoDesc.FormatsRT[1] = formatSpecular;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "PointLight");
		passData->State.Init(dev, psoDesc, schema);

		passData->TransformBuffers.emplace_back(dev, nullptr, static_cast<U32>(sizeof(TransformBuffer)), true);

		const auto volume = Primitive::MakeSphereIco(3);
		passData->VolumeVB.Init(dev, { static_cast<U32>(volume.Vertices.size()), sizeof(Float3), volume.Vertices.data() });
		passData->VolumeIB.Init(dev, { static_cast<U32>(volume.Indices.size()), volume.Indices.data() });

		return passData;
	}

	void Execute(RendererExecuteData& renderData, PassData& passData)
	{
		Data& data = *reinterpret_cast<Data*>(passData.OptData);

		if (data.ShadowData.World.PointLightInfo.Size)
		{
			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<Matrix, TransformBuffer, BUFFER_SHRINK_STEP>(renderData.Dev, data.TransformBuffers, data.ShadowData.World.PointLightInfo.Size);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			const auto& transforms = data.ShadowData.World.ActiveScene->TransformsGlobal;
			const auto& lights = data.ShadowData.World.ActiveScene->PointLightBuffers;

			Resources ids = *passData.Buffers.CastConst<Resources>();
			for (U64 i = 0, j = 0; i < data.ShadowData.World.PointLightInfo.Size; ++j)
			{
				auto& cbuffer = data.TransformBuffers.at(j);
				TransformBuffer* buffer = reinterpret_cast<TransformBuffer*>(cbuffer.GetRegion());

				for (U32 k = 0; k < TransformBuffer::TRANSFORM_COUNT && i < data.ShadowData.World.PointLightInfo.Size; ++k, ++i)
				{
					const auto& transform = transforms[data.ShadowData.World.PointLights[i].TransformIndex];
					ShadowMapCube::Execute(renderData, data.ShadowData, *reinterpret_cast<ShadowMapCube::Resources*>(&ids.ShadowMap), transform.Position);

					const float volume = lights[i].Volume;
					buffer->Transforms[k] = Math::XMMatrixTranspose(Math::XMMatrixScaling(volume, volume, volume) *
						Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&transform.Position))) *
						data.ShadowData.World.DynamicData.ViewProjection;

					renderData.CL.Open(renderData.Dev, data.State);
					ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Point Light nr_" + std::to_wstring(i)).c_str(), Pixel(0xFD, 0xFB, 0xD3));
					renderData.Buffers.BarrierTransition(renderData.CL, ids.ShadowMap, Resource::State::RenderTarget, Resource::State::ShaderResourcePS);

					ctx.BindingSchema.SetGraphics(renderData.CL);
					renderData.Buffers.SetRTV<2>(renderData.CL, &ids.Color, true);

					Resource::Constant<U32> lightBatchId(renderData.Dev, k);
					lightBatchId.Bind(renderData.CL, ctx);
					Resource::Constant<Float3> lightPos(renderData.Dev, transform.Position);
					lightPos.Bind(renderData.CL, ctx);
					lights[i].Buffer.Bind(renderData.CL, ctx);

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