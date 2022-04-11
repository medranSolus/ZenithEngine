#include "GFX/Pipeline/RenderPass/PointLight.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Pipeline/DataPBR.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Primitive.h"

namespace ZE::GFX::Pipeline::RenderPass::PointLight
{
	void Clean(void* data)
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		ShadowMapCube::Clean(execData->ShadowData);
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData,
		PixelFormat formatColor, PixelFormat formatSpecular,
		PixelFormat formatShadow, PixelFormat formatShadowDepth)
	{
		ExecuteData* passData = new ExecuteData;
		ShadowMapCube::Setup(dev, buildData, passData->ShadowData, formatShadowDepth, formatShadow);

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Vertex, Binding::RangeFlag::Constant });
		desc.AddRange({ sizeof(Float3), 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.AddRange({ 1, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 3, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 1, 12, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		const auto& schema = buildData.BindingLib.GetSchema(passData->BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"LightVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"PointLightPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Blender = Resource::BlendType::Light;
		psoDesc.Culling = Resource::CullMode::Front;
		psoDesc.SetDepthClip(false);
		psoDesc.RenderTargetsCount = 2;
		psoDesc.FormatsRT[0] = formatColor;
		psoDesc.FormatsRT[1] = formatSpecular;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "PointLight");
		passData->State.Init(dev, psoDesc, schema);

		passData->TransformBuffers.Exec([&dev](auto& x) { x.emplace_back(dev, nullptr, static_cast<U32>(sizeof(TransformBuffer)), true); });

		const auto volume = Primitive::MakeSphereIcoSolid(3);
		passData->VolumeVB.Init(dev, { static_cast<U32>(volume.Vertices.size()), sizeof(Float3), volume.Vertices.data() });
		passData->VolumeIB.Init(dev, { static_cast<U32>(volume.Indices.size()), volume.Indices.data() });

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);
		data.ShadowData.PreviousEntityCount = 0;

		auto group = Data::GetPointLightGroup(renderData.Registry);
		const U64 count = group.size();
		if (count)
		{
			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<Matrix, TransformBuffer, BUFFER_SHRINK_STEP>(dev, data.TransformBuffers.Get(), count);

			Resources ids = *passData.Buffers.CastConst<Resources>();
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			const Matrix viewProjection = reinterpret_cast<CameraPBR*>(renderData.DynamicData)->ViewProjection;

			for (U64 i = 0, j = 0; i < count; ++j)
			{
				auto& cbuffer = data.TransformBuffers.Get().at(j);
				TransformBuffer* buffer = reinterpret_cast<TransformBuffer*>(cbuffer.GetRegion());

				for (U32 k = 0; k < TransformBuffer::TRANSFORM_COUNT && i < count; ++k, ++i)
				{
					auto entity = group[i];
					const auto& transform = group.get<Data::TransformGlobal>(entity);
					ShadowMapCube::Execute(dev, cl, renderData, data.ShadowData, *reinterpret_cast<ShadowMapCube::Resources*>(&ids.ShadowMap), transform.Position, i);

					const auto& light = group.get<Data::PointLightBuffer>(entity);
					buffer->Transforms[k] = viewProjection *
						Math::XMMatrixTranspose(Math::XMMatrixScaling(light.Volume, light.Volume, light.Volume) *
							Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&transform.Position)));

					cl.Open(dev, data.State);
					ZE_DRAW_TAG_BEGIN(cl, (L"Point Light nr_" + std::to_wstring(i)).c_str(), Pixel(0xFD, 0xFB, 0xD3));
					renderData.Buffers.BarrierTransition(cl, ids.ShadowMap, Resource::State::RenderTarget, Resource::State::ShaderResourcePS);

					ctx.BindingSchema.SetGraphics(cl);
					renderData.Buffers.SetRTV<2>(cl, &ids.Color, true);

					Resource::Constant<U32> lightBatchId(dev, k);
					lightBatchId.Bind(cl, ctx);
					Resource::Constant<Float3> lightPos(dev, transform.Position);
					lightPos.Bind(cl, ctx);
					light.Buffer.Bind(cl, ctx);

					cbuffer.Bind(cl, ctx);
					renderData.Buffers.SetSRV(cl, ctx, ids.ShadowMap);
					renderData.Buffers.SetSRV(cl, ctx, ids.GBufferNormal);
					renderData.DynamicBuffers.Get().Bind(cl, ctx);
					renderData.SettingsBuffer.Bind(cl, ctx);
					ctx.Reset();

					data.VolumeVB.Bind(cl);
					data.VolumeIB.Bind(cl);

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