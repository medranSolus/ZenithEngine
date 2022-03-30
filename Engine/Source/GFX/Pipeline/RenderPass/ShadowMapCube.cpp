#include "GFX/Pipeline/RenderPass/ShadowMapCube.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Vertex.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMapCube
{
	void Setup(Device& dev, RendererBuildData& buildData, ExecuteData& passData, PixelFormat formatDS, PixelFormat formatRT)
	{
		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Vertex, Binding::RangeFlag::Constant });
		desc.AddRange({ sizeof(float), 2, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.AddRange({ sizeof(U32), 1, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.AddRange({ 4, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 1, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 0, Resource::ShaderType::Geometry, Binding::RangeFlag::Constant });
		desc.AddRange({ 1, 1, Resource::ShaderType::Geometry, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 12, Resource::ShaderType::Geometry, Binding::RangeFlag::CBV });
		desc.AddRange({ sizeof(Float3), 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData.BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		const auto& schema = buildData.BindingLib.GetSchema(passData.BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"ShadowCubeDepthVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.GS, L"ShadowCubeDepthGS", buildData.ShaderCache);
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout = Vertex::GetLayout();
		ZE_PSO_SET_NAME(psoDesc, "ShadowMapCubeDepth");
		passData.StateDepth.Init(dev, psoDesc, schema);

		psoDesc.SetShader(psoDesc.VS, L"ShadowCubeVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.GS, L"ShadowCubeGS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"ShadowPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthBefore;
		psoDesc.RenderTargetsCount = 6;
		for (U8 i = 0; i < psoDesc.RenderTargetsCount; ++i)
			psoDesc.FormatsRT[i] = formatRT;
		ZE_PSO_SET_NAME(psoDesc, "ShadowMapCubeSolid");
		passData.StateSolid.Init(dev, psoDesc, schema);

		psoDesc.DepthStencil = Resource::DepthStencilMode::StencilOff;
		ZE_PSO_SET_NAME(psoDesc, "ShadowMapCubeTransparent");
		passData.StateTransparent.Init(dev, psoDesc, schema);

		passData.ViewBuffers.emplace_back(dev, nullptr, static_cast<U32>(sizeof(CubeViewBuffer)), true);
		passData.TransformBuffers.emplace_back(dev, nullptr, static_cast<U32>(sizeof(TransformBuffer)), true);
		passData.Projection = Math::XMMatrixPerspectiveFovLH(static_cast<float>(M_PI_2), 1.0f, 0.01f, 1000.0f);
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData,
		ExecuteData& data, const Resources& ids, const Float3& lightPos, U64 lightNumber)
	{
		// Clearing data on first usage
		cl.Open(dev);
		ZE_DRAW_TAG_BEGIN(cl, L"Shadow Map Cube Clear", PixelVal::Gray);

		renderData.Buffers.ClearDSV(cl, ids.Depth, 1.0f, 0);
		renderData.Buffers.ClearRTV(cl, ids.RenderTarget, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });

		ZE_DRAW_TAG_END(cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);

		auto group = Data::GetRenderGroup<Data::ShadowCaster>(renderData.Registry);
		if (group.size())
		{
			// Resize temporary buffer for view projection data of every light
			const U64 viewBufferIndex = lightNumber / CubeViewBuffer::CUBE_COUNT;
			const U32 viewBufferLightOffset = lightNumber % CubeViewBuffer::CUBE_COUNT;
			Utils::ResizeTransformBuffers<CubeView, CubeViewBuffer>(dev, data.ViewBuffers, lightNumber + 1);

			// Prepare view-projections for casting onto 6 faces
			CubeView& viewBuffer = reinterpret_cast<CubeViewBuffer*>(data.ViewBuffers.at(viewBufferIndex).GetRegion())->Cubes[viewBufferLightOffset];
			const Vector position = Math::XMLoadFloat3(&lightPos);
			const Vector up = { 0.0f, 1.0f, 0.0f, 0.0f };
			// +x
			viewBuffer.View[0] = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 1.0f, 0.0f, 0.0f, 0.0f }, up) * data.Projection);
			// -x
			viewBuffer.View[1] = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ -1.0f, 0.0f, 0.0f, 0.0f }, up) * data.Projection);
			// +y
			viewBuffer.View[2] = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }) * data.Projection);
			// -y
			viewBuffer.View[3] = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 0.0f, -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }) * data.Projection);
			// +z
			viewBuffer.View[4] = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 0.0f, 0.0f, 1.0f, 0.0f }, up) * data.Projection);
			// -z
			viewBuffer.View[5] = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 0.0f, 0.0f, -1.0f, 0.0f }, up) * data.Projection);

			// Sort and split into groups based on materials
			Utils::ViewSortAscending(group, position);
			for (EID entity : group)
			{
				if (renderData.Resources.all_of<Data::MaterialNotSolid>(group.get<Data::MaterialID>(entity).ID))
					renderData.Registry.emplace<Transparent>(entity);
				else
					renderData.Registry.emplace<Solid>(entity);
			}
			auto solidGroup = Data::GetVisibleRenderGroup<Data::ShadowCaster, Solid>(renderData.Registry);
			auto transparentGroup = Data::GetVisibleRenderGroup<Data::ShadowCaster, Transparent>(renderData.Registry);
			const U64 solidCount = solidGroup.size();
			const U64 transparentCount = transparentGroup.size();
			const U64 bufferOffset = data.PreviousEntityCount / TransformBuffer::TRANSFORM_COUNT;
			const U64 bufferTransformOffset = data.PreviousEntityCount % TransformBuffer::TRANSFORM_COUNT;

			// Resize temporary buffer for transform data
			data.PreviousEntityCount += solidCount;
			Utils::ResizeTransformBuffers<Matrix, TransformBuffer, BUFFER_SHRINK_STEP>(dev, data.TransformBuffers, data.PreviousEntityCount + transparentCount);
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			// Depth pre-pass
			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			U64 currentBuffer = bufferOffset;
			U64 currentTransform = bufferTransformOffset;
			Resource::Constant<U32> lightIndex(dev, viewBufferLightOffset);
			for (U64 i = 0; i < solidCount; ++currentBuffer)
			{
				cl.Open(dev, data.StateDepth);
				ZE_DRAW_TAG_BEGIN(cl, (L"Shadow Map Cube Depth Batch_" + std::to_wstring(currentBuffer)).c_str(), Pixel(0x75, 0x7C, 0x88));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetDSV(cl, ids.Depth);

				ctx.SetFromEnd(5);
				auto& cbuffer = data.TransformBuffers.at(currentBuffer);
				cbuffer.Bind(cl, ctx);
				lightIndex.Bind(cl, ctx);
				data.ViewBuffers.at(viewBufferIndex).Bind(cl, ctx);
				renderData.DynamicBuffer.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				TransformBuffer* buffer = reinterpret_cast<TransformBuffer*>(cbuffer.GetRegion());
				for (; currentTransform < TransformBuffer::TRANSFORM_COUNT && i < solidCount; ++currentTransform, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(currentTransform)).c_str(), PixelVal::Gray);

					auto entity = solidGroup[i];
					const auto& transform = solidGroup.get<Data::TransformGlobal>(entity);
					buffer->Transforms[currentTransform] = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

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
				ZE_DRAW_TAG_BEGIN(cl, (L"Shadow Map Cube Solid Batch_" + std::to_wstring(currentBuffer)).c_str(), Pixel(0x52, 0xB2, 0xBF));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(5);
				auto& cbuffer = data.TransformBuffers.at(currentBuffer);
				cbuffer.Bind(cl, ctx);
				lightIndex.Bind(cl, ctx);
				data.ViewBuffers.at(viewBufferIndex).Bind(cl, ctx);
				renderData.DynamicBuffer.Bind(cl, ctx);
				Resource::Constant<Float3> pos(dev, lightPos);
				pos.Bind(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				for (; currentTransform < TransformBuffer::TRANSFORM_COUNT && i < solidCount; ++currentTransform, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(currentTransform)).c_str(), Pixel(0x01, 0x60, 0x64));

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
			currentBuffer = data.PreviousEntityCount / TransformBuffer::TRANSFORM_COUNT;
			currentTransform = data.PreviousEntityCount % TransformBuffer::TRANSFORM_COUNT;
			data.PreviousEntityCount += transparentCount;
			for (U64 i = 0; i < transparentCount; ++currentBuffer)
			{
				cl.Open(dev, data.StateTransparent);
				ZE_DRAW_TAG_BEGIN(cl, (L"Shadow Map Cube Transparent Batch_" + std::to_wstring(currentBuffer)).c_str(), Pixel(0x52, 0xB2, 0xBF));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(5);
				auto& cbuffer = data.TransformBuffers.at(currentBuffer);
				cbuffer.Bind(cl, ctx);
				lightIndex.Bind(cl, ctx);
				data.ViewBuffers.at(viewBufferIndex).Bind(cl, ctx);
				renderData.DynamicBuffer.Bind(cl, ctx);
				Resource::Constant<Float3> pos(dev, lightPos);
				pos.Bind(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				TransformBuffer* buffer = reinterpret_cast<TransformBuffer*>(cbuffer.GetRegion());
				for (; currentTransform < TransformBuffer::TRANSFORM_COUNT && i < transparentCount; ++currentTransform, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(currentTransform)).c_str(), Pixel(0x01, 0x60, 0x64));

					EID entity = transparentGroup[i];
					const auto& transform = transparentGroup.get<Data::TransformGlobal>(entity);
					buffer->Transforms[currentTransform] = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

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
			// Remove current material indication
			renderData.Registry.clear<Solid, Transparent>();
		}
	}
}