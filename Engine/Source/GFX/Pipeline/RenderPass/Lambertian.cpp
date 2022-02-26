#include "GFX/Pipeline/RenderPass/Lambertian.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::Lambertian
{
	Data* Setup(Device& dev, RendererBuildData& buildData, Info::World& worldData, PixelFormat formatDS,
		PixelFormat formatColor, PixelFormat formatNormal, PixelFormat formatSpecular)
	{
		Data* passData = new Data{ worldData };

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Vertex, Binding::RangeFlag::Constant });
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
		desc.AddRange({ 4, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 1, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 12, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Vertex | Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		const auto& schema = buildData.BindingLib.GetSchema(passData->BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"PhongDepthVS", buildData.ShaderCache);
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Normal);
		psoDesc.InputLayout.emplace_back(Resource::InputParam::TexCoord);
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Bitangent);
		ZE_PSO_SET_NAME(psoDesc, "LambertianDepth");
		passData->StateDepth.Init(dev, psoDesc, schema);

		psoDesc.SetShader(psoDesc.VS, L"PhongVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"PhongPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthBefore;
		psoDesc.RenderTargetsCount = 3;
		psoDesc.FormatsRT[0] = formatColor;
		psoDesc.FormatsRT[1] = formatNormal;
		psoDesc.FormatsRT[2] = formatSpecular;
		ZE_PSO_SET_NAME(psoDesc, "Lambertian");
		passData->StateNormal.Init(dev, psoDesc, schema);

		passData->TransformBuffers.emplace_back(dev, nullptr, static_cast<U32>(sizeof(ModelTransformBuffer)), true);

		return passData;
	}

	void Execute(RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		Data& data = *reinterpret_cast<Data*>(passData.OptData);

		// Clearing data on first usage
		renderData.CL.Open(renderData.Dev);
		ZE_DRAW_TAG_BEGIN(renderData.CL, L"Lambertian Clear", PixelVal::White);

		renderData.Buffers.ClearDSV(renderData.CL, ids.DepthStencil, 1.0f, 0);
		renderData.Buffers.ClearRTV(renderData.CL, ids.Color, ColorF4());
		renderData.Buffers.ClearRTV(renderData.CL, ids.Normal, ColorF4());
		renderData.Buffers.ClearRTV(renderData.CL, ids.Specular, ColorF4());

		ZE_DRAW_TAG_END(renderData.CL);
		renderData.CL.Close(renderData.Dev);
		renderData.Dev.ExecuteMain(renderData.CL);

		if (data.World.MeshInfo.Size)
		{
			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<ModelTransform, ModelTransformBuffer, BUFFER_SHRINK_STEP>(renderData.Dev, data.TransformBuffers, data.World.MeshInfo.Size);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			const auto& transforms = data.World.ActiveScene->TransformsGlobal;
			const auto& geometries = data.World.ActiveScene->Geometries;

			// Depth pre-pass
			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			for (U64 i = 0, j = 0; i < data.World.MeshInfo.Size; ++j)
			{
				renderData.CL.Open(renderData.Dev, data.StateDepth);
				ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Lambertian Depth Batch_" + std::to_wstring(j)).c_str(), Pixel(0xC2, 0xC5, 0xCC));
				ctx.BindingSchema.SetGraphics(renderData.CL);
				renderData.Buffers.SetDSV(renderData.CL, ids.DepthStencil);

				ctx.SetFromEnd(1);
				auto& cbuffer = data.TransformBuffers.at(j);
				cbuffer.Bind(renderData.CL, ctx);
				ctx.Reset();

				// Compute single batch
				ModelTransformBuffer* buffer = reinterpret_cast<ModelTransformBuffer*>(cbuffer.GetRegion());
				for (U32 k = 0; k < ModelTransformBuffer::TRANSFORM_COUNT && i < data.World.MeshInfo.Size; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Mesh_" + std::to_wstring(k)).c_str(), PixelVal::Gray);

					const auto& info = data.World.Meshes[i];
					const auto& transform = transforms[info.TransformIndex];
					buffer->Transforms[k].Model = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
					buffer->Transforms[k].ModelViewProjection = buffer->Transforms[k].Model * data.World.DynamicData.ViewProjection;

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
			U64 currentMaterialIndex = UINT64_MAX;
			// Normal pass
			for (U64 i = 0, j = 0; i < data.World.MeshInfo.Size; ++j)
			{
				renderData.CL.Open(renderData.Dev, data.StateNormal);
				ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Lambertian Batch_" + std::to_wstring(j)).c_str(), Pixel(0xC2, 0xC5, 0xCC));
				ctx.BindingSchema.SetGraphics(renderData.CL);
				renderData.Buffers.SetOutput<3>(renderData.CL, &ids.Color, ids.DepthStencil, true);

				ctx.SetFromEnd(1);
				data.TransformBuffers.at(j).Bind(renderData.CL, ctx);
				data.World.DynamicDataBuffer.Bind(renderData.CL, ctx);
				renderData.EngineData.Bind(renderData.CL, ctx);
				ctx.Reset();

				// Compute single batch
				for (U32 k = 0; k < TransformBuffer::TRANSFORM_COUNT && i < data.World.MeshInfo.Size; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Mesh_" + std::to_wstring(k)).c_str(), Pixel(0xAD, 0xAD, 0xC9));
					Resource::Constant<U32> meshBatchId(renderData.Dev, k);
					meshBatchId.Bind(renderData.CL, ctx);

					const auto& info = data.World.Meshes[i];
					if (currentMaterialIndex != info.MaterialIndex)
					{
						currentMaterialIndex = info.MaterialIndex;
						const auto& material = materials[currentMaterialIndex];
						material.BindBuffer(renderData.CL, ctx);
						material.BindTextures(renderData.CL, ctx);
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
	}
}