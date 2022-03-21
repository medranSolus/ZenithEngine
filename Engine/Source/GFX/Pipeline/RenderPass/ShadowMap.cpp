#include "GFX/Pipeline/RenderPass/ShadowMap.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Vertex.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMap
{
	void Setup(Device& dev, RendererBuildData& buildData, ExecuteData& passData,
		PixelFormat formatDS, PixelFormat formatRT, Matrix&& projection)
	{
		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Vertex, Binding::RangeFlag::Constant });
		desc.AddRange({ sizeof(float), 2, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.AddRange({ sizeof(U32), 1, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.AddRange({ 4, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 1, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 12, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.AddRange({ sizeof(Float3), 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData.BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		const auto& schema = buildData.BindingLib.GetSchema(passData.BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"PhongDepthVS", buildData.ShaderCache);
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout = Vertex::GetLayout();
		ZE_PSO_SET_NAME(psoDesc, "ShadowMapDepth");
		passData.StateDepth.Init(dev, psoDesc, schema);

		psoDesc.SetShader(psoDesc.VS, L"PhongVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"ShadowPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthBefore;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		ZE_PSO_SET_NAME(psoDesc, "ShadowMap");
		passData.StateNormal.Init(dev, psoDesc, schema);

		passData.TransformBuffers.emplace_back(dev, nullptr, static_cast<U32>(sizeof(ModelTransformBuffer)), true);
		passData.Projection = std::move(projection);
	}

	Matrix Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData,
		ExecuteData& data, const Resources& ids, const Float3& lightPos, const Float3& lightDir)
	{
		// Clearing data on first usage
		cl.Open(dev);
		ZE_DRAW_TAG_BEGIN(cl, L"Shadow Map Clear", PixelVal::Gray);

		renderData.Buffers.ClearDSV(cl, ids.Depth, 1.0f, 0);
		renderData.Buffers.ClearRTV(cl, ids.RenderTarget, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });

		ZE_DRAW_TAG_END(cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);

		// Prepare view-projection for shadow
		const Vector position = Math::XMLoadFloat3(&lightPos);
		const Vector direction = Math::XMLoadFloat3(&lightDir);
		const Vector up = Math::XMVector3TransformNormal({ 0.0f, 0.0f, 1.0f, 0.0f },
			Math::GetVectorRotation({ 0.0f, -1.0f, 0.0f, 0.0f }, direction));
		Matrix viewProjection = Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position, Math::XMVectorAdd(position, direction), up) * data.Projection);

		auto group = Data::GetRenderGroup<Data::ShadowCaster>(renderData.Registry);
		const U64 count = group.size();
		if (count)
		{
			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<ModelTransform, ModelTransformBuffer, BUFFER_SHRINK_STEP>(dev, data.TransformBuffers, count);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			// Depth pre-pass
			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			for (U64 i = 0, j = 0; i < count; ++j)
			{
				cl.Open(dev, data.StateDepth);
				ZE_DRAW_TAG_BEGIN(cl, (L"Shadow Map Depth Batch_" + std::to_wstring(j)).c_str(), Pixel(0x98, 0x9F, 0xA7));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetDSV(cl, ids.Depth);

				ctx.SetFromEnd(3);
				auto& cbuffer = data.TransformBuffers.at(j);
				cbuffer.Bind(cl, ctx);
				renderData.DynamicBuffer.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				ModelTransformBuffer* buffer = reinterpret_cast<ModelTransformBuffer*>(cbuffer.GetRegion());
				for (U32 k = 0; k < ModelTransformBuffer::TRANSFORM_COUNT && i < count; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(k)).c_str(), PixelVal::Gray);

					auto entity = group[i];
					const auto& transform = group.get<Data::TransformGlobal>(entity);
					buffer->Transforms[k].Model = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
					buffer->Transforms[k].ModelViewProjection = viewProjection * buffer->Transforms[k].Model;

					Resource::Constant<U32> meshBatchId(dev, k);
					meshBatchId.Bind(cl, ctx);
					ctx.Reset();

					const auto& geometry = renderData.Resources.get<Data::Geometry>(group.get<Data::MeshID>(entity).ID);
					geometry.Vertices.Bind(cl);
					geometry.Indices.Bind(cl);

					cl.DrawIndexed(dev, geometry.Indices.GetCount());
					ZE_DRAW_TAG_END(cl);
				}

				ZE_DRAW_TAG_END(cl);
				cl.Close(dev);
				dev.ExecuteMain(cl);
			}

			Data::MaterialID currentMaterial = { INVALID_EID };
			// Normal pass
			for (U64 i = 0, j = 0; i < count; ++j)
			{
				cl.Open(dev, data.StateNormal);
				ZE_DRAW_TAG_BEGIN(cl, (L"Shadow Map Batch_" + std::to_wstring(j)).c_str(), Pixel(0x79, 0x82, 0x8D));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(3);
				auto& cbuffer = data.TransformBuffers.at(j);
				cbuffer.Bind(cl, ctx);
				renderData.DynamicBuffer.Bind(cl, ctx);
				Resource::Constant<Float3> pos(dev, lightPos);
				pos.Bind(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				for (U32 k = 0; k < ModelTransformBuffer::TRANSFORM_COUNT && i < count; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(k)).c_str(), Pixel(0x5D, 0x5E, 0x61));

					Resource::Constant<U32> meshBatchId(dev, k);
					meshBatchId.Bind(cl, ctx);

					auto entity = group[i];
					auto material = group.get<Data::MaterialID>(entity);
					if (currentMaterial.ID != material.ID)
					{
						currentMaterial = material;

						const auto& matData = renderData.Resources.get<Data::MaterialPBR>(material.ID);
						Resource::Constant<float> parallaxScale(dev, matData.ParallaxScale);
						parallaxScale.Bind(cl, ctx);
						Resource::Constant<U32> materialFlags(dev, matData.Flags);
						materialFlags.Bind(cl, ctx);
						renderData.Resources.get<Data::MaterialBuffersPBR>(material.ID).BindTextures(cl, ctx);
					}
					ctx.Reset();

					const auto& geometry = renderData.Resources.get<Data::Geometry>(group.get<Data::MeshID>(entity).ID);
					geometry.Vertices.Bind(cl);
					geometry.Indices.Bind(cl);

					cl.DrawIndexed(dev, geometry.Indices.GetCount());
					ZE_DRAW_TAG_END(cl);
				}

				ZE_DRAW_TAG_END(cl);
				cl.Close(dev);
				dev.ExecuteMain(cl);
			}
		}
		return viewProjection;
	}
}