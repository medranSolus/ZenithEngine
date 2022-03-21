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
		desc.AddRange({ 1, 0, Resource::ShaderType::Geometry, Binding::RangeFlag::CBV });
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
		ZE_PSO_SET_NAME(psoDesc, "ShadowMapCube");
		passData.StateNormal.Init(dev, psoDesc, schema);

		passData.ViewBuffer.Init(dev, nullptr, static_cast<U32>(sizeof(Matrix) * 6), true);
		passData.TransformBuffers.emplace_back(dev, nullptr, static_cast<U32>(sizeof(TransformBuffer)), true);
		passData.Projection = Math::XMMatrixPerspectiveFovLH(static_cast<float>(M_PI_2), 1.0f, 0.01f, 1000.0f);
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData,
		ExecuteData& data, const Resources& ids, const Float3& lightPos)
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
		const U64 count = group.size();
		if (count)
		{
			// Prepare view-projections for casting onto 6 faces
			Matrix* viewBuffer = reinterpret_cast<Matrix*>(data.ViewBuffer.GetRegion());
			const Vector position = Math::XMLoadFloat3(&lightPos);
			const Vector up = { 0.0f, 1.0f, 0.0f, 0.0f };
			// +x
			viewBuffer[0] = Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position,
				Math::XMVectorAdd(position, { 1.0f, 0.0f, 0.0f, 0.0f }), up) * data.Projection);
			// -x
			viewBuffer[1] = Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position,
				Math::XMVectorAdd(position, { -1.0f, 0.0f, 0.0f, 0.0f }), up) * data.Projection);
			// +y
			viewBuffer[2] = Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position,
				Math::XMVectorAdd(position, { 0.0f, 1.0f, 0.0f, 0.0f }),
				{ 0.0f, 0.0f, -1.0f, 0.0f }) * data.Projection);
			// -y
			viewBuffer[3] = Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position,
				Math::XMVectorAdd(position, { 0.0f, -1.0f, 0.0f, 0.0f }),
				{ 0.0f, 0.0f, 1.0f, 0.0f }) * data.Projection);
			// +z
			viewBuffer[4] = Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position,
				Math::XMVectorAdd(position, { 0.0f, 0.0f, 1.0f, 0.0f }), up) * data.Projection);
			// -z
			viewBuffer[5] = Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position,
				Math::XMVectorAdd(position, { 0.0f, 0.0f, -1.0f, 0.0f }), up) * data.Projection);

			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<Matrix, TransformBuffer, BUFFER_SHRINK_STEP>(dev, data.TransformBuffers, count);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			// Depth pre-pass
			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			for (U64 i = 0, j = 0; i < count; ++j)
			{
				cl.Open(dev, data.StateDepth);
				ZE_DRAW_TAG_BEGIN(cl, (L"Shadow Map Cube Depth Batch_" + std::to_wstring(j)).c_str(), Pixel(0x75, 0x7C, 0x88));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetDSV(cl, ids.Depth);

				ctx.SetFromEnd(4);
				auto& cbuffer = data.TransformBuffers.at(j);
				cbuffer.Bind(cl, ctx);
				data.ViewBuffer.Bind(cl, ctx);
				renderData.DynamicBuffer.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				TransformBuffer* buffer = reinterpret_cast<TransformBuffer*>(cbuffer.GetRegion());
				for (U32 k = 0; k < TransformBuffer::TRANSFORM_COUNT && i < count; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(k)).c_str(), PixelVal::Gray);

					auto entity = group[i];
					const auto& transform = group.get<Data::TransformGlobal>(entity);
					buffer->Transforms[k] = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

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
				ZE_DRAW_TAG_BEGIN(cl, (L"Shadow Map Cube Batch_" + std::to_wstring(j)).c_str(), Pixel(0x52, 0xB2, 0xBF));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(4);
				auto& cbuffer = data.TransformBuffers.at(j);
				cbuffer.Bind(cl, ctx);
				data.ViewBuffer.Bind(cl, ctx);
				renderData.DynamicBuffer.Bind(cl, ctx);
				Resource::Constant<Float3> pos(dev, lightPos);
				pos.Bind(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				for (U32 k = 0; k < TransformBuffer::TRANSFORM_COUNT && i < count; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(k)).c_str(), Pixel(0x01, 0x60, 0x64));

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
	}
}