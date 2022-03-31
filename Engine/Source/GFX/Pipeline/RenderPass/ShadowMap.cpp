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
		ZE_PSO_SET_NAME(psoDesc, "ShadowMapSolid");
		passData.StateSolid.Init(dev, psoDesc, schema);

		psoDesc.DepthStencil = Resource::DepthStencilMode::StencilOff;
		ZE_PSO_SET_NAME(psoDesc, "ShadowMapTransparent");
		passData.StateTransparent.Init(dev, psoDesc, schema);

		passData.TransformBuffers.Exec([&dev](auto& x) { x.emplace_back(dev, nullptr, static_cast<U32>(sizeof(ModelTransformBuffer)), true); });
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
		Matrix viewProjection = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position, direction, Math::XMVector3Orthogonal(direction)) * data.Projection);

		auto group = Data::GetRenderGroup<Data::ShadowCaster>(renderData.Registry);
		if (group.size())
		{
			// Compute visibility of objects inside camera view
			Math::BoundingFrustum frustum(data.Projection, false);
			frustum.Transform(frustum, 1.0f, Math::XMQuaternionRotationMatrix(Math::GetVectorRotation({ 0.0f, 0.0f, 1.0f, 0.0f }, direction)), position);
			Utils::FrustumCulling<InsideFrustumSolid, InsideFrustumNotSolid>(renderData.Registry, renderData.Resources, group, frustum);

			// Use new group visible only in current frustum and sort
			Utils::ViewSortAscending(group, position);
			auto solidGroup = Data::GetVisibleRenderGroup<Data::ShadowCaster, InsideFrustumSolid>(renderData.Registry);
			auto transparentGroup = Data::GetVisibleRenderGroup<Data::ShadowCaster, InsideFrustumNotSolid>(renderData.Registry);
			const U64 solidCount = solidGroup.size();
			const U64 transparentCount = transparentGroup.size();
			const U64 bufferOffset = data.PreviousEntityCount / ModelTransformBuffer::TRANSFORM_COUNT;
			const U64 bufferTransformOffset = data.PreviousEntityCount % ModelTransformBuffer::TRANSFORM_COUNT;

			// Resize temporary buffer for transform data
			data.PreviousEntityCount += solidCount;
			Utils::ResizeTransformBuffers<ModelTransform, ModelTransformBuffer, BUFFER_SHRINK_STEP>(dev, data.TransformBuffers.Get(), data.PreviousEntityCount + transparentCount);
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			// Depth pre-pass
			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			U64 currentBuffer = bufferOffset;
			U64 currentTransform = bufferTransformOffset;
			for (U64 i = 0; i < solidCount; ++currentBuffer)
			{
				cl.Open(dev, data.StateDepth);
				ZE_DRAW_TAG_BEGIN(cl, (L"Shadow Map Depth Batch_" + std::to_wstring(currentBuffer)).c_str(), Pixel(0x98, 0x9F, 0xA7));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetDSV(cl, ids.Depth);

				ctx.SetFromEnd(3);
				auto& cbuffer = data.TransformBuffers.Get().at(currentBuffer);
				cbuffer.Bind(cl, ctx);
				renderData.DynamicBuffers.Get().Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				ModelTransformBuffer* buffer = reinterpret_cast<ModelTransformBuffer*>(cbuffer.GetRegion());
				for (; currentTransform < ModelTransformBuffer::TRANSFORM_COUNT && i < solidCount; ++currentTransform, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(currentTransform)).c_str(), PixelVal::Gray);

					auto entity = solidGroup[i];
					const auto& transform = solidGroup.get<Data::TransformGlobal>(entity);
					buffer->Transforms[currentTransform].Model = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
					buffer->Transforms[currentTransform].ModelViewProjection = viewProjection * buffer->Transforms[currentTransform].Model;

					Resource::Constant<U32> meshBatchId(dev, currentTransform);
					meshBatchId.Bind(cl, ctx);
					ctx.Reset();

					const auto& geometry = renderData.Resources.get<Data::Geometry>(solidGroup.get<Data::MeshID>(entity).ID);
					geometry.Vertices.Bind(cl);
					geometry.Indices.Bind(cl);

					cl.DrawIndexed(dev, geometry.Indices.GetCount());
					ZE_DRAW_TAG_END(cl);
				}

				ZE_DRAW_TAG_END(cl);
				cl.Close(dev);
				dev.ExecuteMain(cl);
				currentTransform = 0;
			}

			// Solid pass
			Data::MaterialID currentMaterial = { INVALID_EID };
			currentBuffer = bufferOffset;
			currentTransform = bufferTransformOffset;
			for (U64 i = 0; i < solidCount; ++currentBuffer)
			{
				cl.Open(dev, data.StateSolid);
				ZE_DRAW_TAG_BEGIN(cl, (L"Shadow Map Solid Batch_" + std::to_wstring(currentBuffer)).c_str(), Pixel(0x79, 0x82, 0x8D));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(3);
				auto& cbuffer = data.TransformBuffers.Get().at(currentBuffer);
				cbuffer.Bind(cl, ctx);
				renderData.DynamicBuffers.Get().Bind(cl, ctx);
				Resource::Constant<Float3> pos(dev, lightPos);
				pos.Bind(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				for (; currentTransform < ModelTransformBuffer::TRANSFORM_COUNT && i < solidCount; ++currentTransform, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(currentTransform)).c_str(), Pixel(0x5D, 0x5E, 0x61));

					Resource::Constant<U32> meshBatchId(dev, currentTransform);
					meshBatchId.Bind(cl, ctx);

					auto entity = solidGroup[i];
					auto material = solidGroup.get<Data::MaterialID>(entity);
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

					const auto& geometry = renderData.Resources.get<Data::Geometry>(solidGroup.get<Data::MeshID>(entity).ID);
					geometry.Vertices.Bind(cl);
					geometry.Indices.Bind(cl);

					cl.DrawIndexed(dev, geometry.Indices.GetCount());
					ZE_DRAW_TAG_END(cl);
				}

				ZE_DRAW_TAG_END(cl);
				cl.Close(dev);
				dev.ExecuteMain(cl);
				currentTransform = 0;
				currentMaterial = { INVALID_EID };
			}

			// Transparent pass
			currentBuffer = data.PreviousEntityCount / ModelTransformBuffer::TRANSFORM_COUNT;
			currentTransform = data.PreviousEntityCount % ModelTransformBuffer::TRANSFORM_COUNT;
			data.PreviousEntityCount += transparentCount;
			for (U64 i = 0; i < transparentCount; ++currentBuffer)
			{
				cl.Open(dev, data.StateTransparent);
				ZE_DRAW_TAG_BEGIN(cl, (L"Shadow Map Transparent Batch_" + std::to_wstring(currentBuffer)).c_str(), Pixel(0x79, 0x82, 0x8D));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(3);
				auto& cbuffer = data.TransformBuffers.Get().at(currentBuffer);
				cbuffer.Bind(cl, ctx);
				renderData.DynamicBuffers.Get().Bind(cl, ctx);
				Resource::Constant<Float3> pos(dev, lightPos);
				pos.Bind(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				ModelTransformBuffer* buffer = reinterpret_cast<ModelTransformBuffer*>(cbuffer.GetRegion());
				for (; currentTransform < ModelTransformBuffer::TRANSFORM_COUNT && i < transparentCount; ++currentTransform, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(currentTransform)).c_str(), Pixel(0x5D, 0x5E, 0x61));

					EID entity = transparentGroup[i];
					const auto& transform = transparentGroup.get<Data::TransformGlobal>(entity);
					buffer->Transforms[currentTransform].Model = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
					buffer->Transforms[currentTransform].ModelViewProjection = viewProjection * buffer->Transforms[currentTransform].Model;

					Resource::Constant<U32> meshBatchId(dev, currentTransform);
					meshBatchId.Bind(cl, ctx);

					auto material = transparentGroup.get<Data::MaterialID>(entity);
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

					const auto& geometry = renderData.Resources.get<Data::Geometry>(transparentGroup.get<Data::MeshID>(entity).ID);
					geometry.Vertices.Bind(cl);
					geometry.Indices.Bind(cl);

					cl.DrawIndexed(dev, geometry.Indices.GetCount());
					ZE_DRAW_TAG_END(cl);
				}

				ZE_DRAW_TAG_END(cl);
				cl.Close(dev);
				dev.ExecuteMain(cl);
				currentTransform = 0;
				currentMaterial = { INVALID_EID };
			}
			// Remove current visibility indication
			renderData.Registry.clear<InsideFrustumSolid, InsideFrustumNotSolid>();
		}
		return viewProjection;
	}
}