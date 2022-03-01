#include "GFX/Pipeline/RenderPass/ShadowMap.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMap
{
	void Setup(Device& dev, RendererBuildData& buildData, Data& passData,
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
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Normal);
		psoDesc.InputLayout.emplace_back(Resource::InputParam::TexCoord);
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Bitangent);
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

	Matrix Execute(RendererExecuteData& renderData, Data& data, const Resources& ids,
		const Float3& lightPos, const Float3& lightDir)
	{
		// Prepare view-projection for shadow
		const Vector position = Math::XMLoadFloat3(&lightPos);
		const Vector direction = Math::XMLoadFloat3(&lightDir);
		const Vector up = Math::XMVector3TransformNormal({ 0.0f, 0.0f, 1.0f, 0.0f },
			Math::GetVectorRotation({ 0.0f, -1.0f, 0.0f, 0.0f }, direction));
		Matrix viewProjection = Math::XMMatrixTranspose(Math::XMMatrixLookAtLH(position, Math::XMVectorAdd(position, direction), up) * data.Projection);

		if (data.World.ShadowCasterInfo.Size)
		{
			// Clearing data on first usage
			renderData.CL.Open(renderData.Dev);
			ZE_DRAW_TAG_BEGIN(renderData.CL, L"Shadow Map Clear", PixelVal::Gray);

			renderData.Buffers.ClearDSV(renderData.CL, ids.Depth, 1.0f, 0);
			renderData.Buffers.ClearRTV(renderData.CL, ids.RenderTarget, ColorF4());

			ZE_DRAW_TAG_END(renderData.CL);
			renderData.CL.Close(renderData.Dev);
			renderData.Dev.ExecuteMain(renderData.CL);

			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<ModelTransform, ModelTransformBuffer, BUFFER_SHRINK_STEP>(renderData.Dev, data.TransformBuffers, data.World.ShadowCasterInfo.Size);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			const auto& transforms = data.World.ActiveScene->TransformsGlobal;
			const auto& geometries = data.World.ActiveScene->Geometries;

			// Depth pre-pass
			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			for (U64 i = 0, j = 0; i < data.World.ShadowCasterInfo.Size; ++j)
			{
				renderData.CL.Open(renderData.Dev, data.StateDepth);
				ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Shadow Map Depth Batch_" + std::to_wstring(j)).c_str(), Pixel(0x98, 0x9F, 0xA7));
				ctx.BindingSchema.SetGraphics(renderData.CL);
				renderData.Buffers.SetDSV(renderData.CL, ids.Depth);

				ctx.SetFromEnd(3);
				auto& cbuffer = data.TransformBuffers.at(j);
				cbuffer.Bind(renderData.CL, ctx);
				data.World.DynamicDataBuffer.Bind(renderData.CL, ctx);
				ctx.Reset();

				// Compute single batch
				ModelTransformBuffer* buffer = reinterpret_cast<ModelTransformBuffer*>(cbuffer.GetRegion());
				for (U32 k = 0; k < ModelTransformBuffer::TRANSFORM_COUNT && i < data.World.ShadowCasterInfo.Size; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Mesh_" + std::to_wstring(k)).c_str(), PixelVal::Gray);

					const auto& info = data.World.Meshes[i];
					const auto& transform = transforms[info.TransformIndex];
					buffer->Transforms[k].Model = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
					buffer->Transforms[k].ModelViewProjection = viewProjection * buffer->Transforms[k].Model;

					Resource::Constant<U32> meshBatchId(renderData.Dev, k);
					meshBatchId.Bind(renderData.CL, ctx);
					ctx.Reset();

					const auto& geometry = geometries[info.GeometryIndex];
					geometry.Vertices.Bind(renderData.CL);
					geometry.Indices.Bind(renderData.CL);

					renderData.CL.DrawIndexed(renderData.Dev, geometry.Indices.GetCount());
					ZE_DRAW_TAG_END(renderData.CL);
				}

				ZE_DRAW_TAG_END(renderData.CL);
				renderData.CL.Close(renderData.Dev);
				renderData.Dev.ExecuteMain(renderData.CL);
			}

			const auto& materials = data.World.ActiveScene->Materials;
			const auto& materialBufferss = data.World.ActiveScene->MaterialBuffers;
			U64 currentMaterialIndex = UINT64_MAX;
			// Normal pass
			for (U64 i = 0, j = 0; i < data.World.ShadowCasterInfo.Size; ++j)
			{
				renderData.CL.Open(renderData.Dev, data.StateNormal);
				ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Shadow Map Batch_" + std::to_wstring(j)).c_str(), Pixel(0x79, 0x82, 0x8D));
				ctx.BindingSchema.SetGraphics(renderData.CL);
				renderData.Buffers.SetOutput(renderData.CL, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(3);
				auto& cbuffer = data.TransformBuffers.at(j);
				cbuffer.Bind(renderData.CL, ctx);
				data.World.DynamicDataBuffer.Bind(renderData.CL, ctx);
				Resource::Constant<Float3> pos(renderData.Dev, lightPos);
				pos.Bind(renderData.CL, ctx);
				renderData.EngineData.Bind(renderData.CL, ctx);
				ctx.Reset();

				// Compute single batch
				for (U32 k = 0; k < ModelTransformBuffer::TRANSFORM_COUNT && i < data.World.ShadowCasterInfo.Size; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Mesh_" + std::to_wstring(k)).c_str(), Pixel(0x5D, 0x5E, 0x61));

					Resource::Constant<U32> meshBatchId(renderData.Dev, k);
					meshBatchId.Bind(renderData.CL, ctx);

					const auto& info = data.World.ShadowCasters[i];
					if (currentMaterialIndex != info.MaterialIndex)
					{
						currentMaterialIndex = info.MaterialIndex;
						const auto& material = materials[currentMaterialIndex];

						Resource::Constant<float> parallaxScale(renderData.Dev, material.ParallaxScale);
						parallaxScale.Bind(renderData.CL, ctx);
						Resource::Constant<U32> materialFlags(renderData.Dev, material.Flags);
						materialFlags.Bind(renderData.CL, ctx);
						materialBufferss[currentMaterialIndex].BindTextures(renderData.CL, ctx);
					}
					ctx.Reset();

					const auto& geometry = geometries[info.GeometryIndex];
					geometry.Vertices.Bind(renderData.CL);
					geometry.Indices.Bind(renderData.CL);

					renderData.CL.DrawIndexed(renderData.Dev, geometry.Indices.GetCount());
					ZE_DRAW_TAG_END(renderData.CL);
				}

				ZE_DRAW_TAG_END(renderData.CL);
				renderData.CL.Close(renderData.Dev);
				renderData.Dev.ExecuteMain(renderData.CL);
			}
		}
		return viewProjection;
	}
}