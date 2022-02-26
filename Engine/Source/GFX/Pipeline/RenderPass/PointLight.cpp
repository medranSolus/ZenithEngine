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

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Vertex, Binding::RangeFlag::Constant });
		desc.AddRange({ sizeof(Float3), 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.AddRange({ 1, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 3, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 1, 12, Resource::ShaderType::Vertex | Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

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
		passData->VolumeVB.Init(dev, { static_cast<U32>(volume.first.size()), sizeof(Float3), volume.first.data() });
		passData->VolumeIB.Init(dev, { static_cast<U32>(volume.second.size()), volume.second.data() });

		return passData;
	}

	void Execute(RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		Data& data = *reinterpret_cast<Data*>(passData.OptData);

		// Clearing data on first usage
		renderData.CL.Open(renderData.Dev);
		ZE_DRAW_TAG_BEGIN(renderData.CL, L"Lighting Clear", PixelVal::White);

		renderData.Buffers.ClearRTV(renderData.CL, ids.Color, ColorF4());
		renderData.Buffers.ClearRTV(renderData.CL, ids.Specular, ColorF4());

		ZE_DRAW_TAG_END(renderData.CL);
		renderData.CL.Close(renderData.Dev);
		renderData.Dev.ExecuteMain(renderData.CL);

		if (data.World.PointLightInfo.Size)
		{
			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<Matrix, TransformBuffer, BUFFER_SHRINK_STEP>(renderData.Dev, data.TransformBuffers, data.World.PointLightInfo.Size);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			const auto& transforms = data.World.ActiveScene->TransformsGlobal;
			const auto& lights = data.World.ActiveScene->PointLightBuffers;

			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			for (U64 i = 0, j = 0; i < data.World.PointLightInfo.Size; ++j)
			{
				renderData.CL.Open(renderData.Dev, data.State);
				ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Point Light Batch_" + std::to_wstring(j)).c_str(), Pixel(0xFD, 0xFB, 0xD3));
				ctx.BindingSchema.SetGraphics(renderData.CL);
				renderData.Buffers.SetRTV<2>(renderData.CL, &ids.Color, true);

				ctx.SetFromEnd(4);
				auto& cbuffer = data.TransformBuffers.at(j);
				cbuffer.Bind(renderData.CL, ctx);
				renderData.Buffers.SetSRV(renderData.CL, ctx, ids.ShadowMap);
				renderData.Buffers.SetSRV(renderData.CL, ctx, ids.GBufferNormal);
				data.World.DynamicDataBuffer.Bind(renderData.CL, ctx);
				renderData.EngineData.Bind(renderData.CL, ctx);
				ctx.Reset();

				// Compute single batch
				TransformBuffer* buffer = reinterpret_cast<TransformBuffer*>(cbuffer.GetRegion());
				for (U32 k = 0; k < TransformBuffer::TRANSFORM_COUNT && i < data.World.PointLightInfo.Size; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Light_" + std::to_wstring(k)).c_str(), Pixel(0xF4, 0xE9, 0x9B));

					const auto& transform = transforms[data.World.PointLights[i].TransformIndex];
					const float volume = lights[i].Volume;
					buffer->Transforms[k] = Math::XMMatrixTranspose(Math::XMMatrixScaling(volume, volume, volume) *
						Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&transform.Position))) *
						data.World.DynamicData.ViewProjection;

					Resource::Constant<U32> lightBatchId(renderData.Dev, k);
					lightBatchId.Bind(renderData.CL, ctx);
					Resource::Constant<Float3> lightPos(renderData.Dev, transform.Position);
					lightPos.Bind(renderData.CL, ctx);
					lights[i].Buffer.Bind(renderData.CL, ctx);
					ctx.Reset();

					data.VolumeVB.Bind(renderData.CL);
					data.VolumeIB.Bind(renderData.CL);

					renderData.CL.DrawIndexed(renderData.Dev, data.VolumeIB.GetCount());
					ZE_DRAW_TAG_END(renderData.CL);
				}

				ZE_DRAW_TAG_END(renderData.CL);
				renderData.CL.Close(renderData.Dev);
				renderData.Dev.ExecuteMain(renderData.CL);
			}
		}
	}
}