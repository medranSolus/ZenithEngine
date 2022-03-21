#include "GFX/Pipeline/RenderPass/Lambertian.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Pipeline/DataPBR.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Vertex.h"

namespace ZE::GFX::Pipeline::RenderPass::Lambertian
{
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatDS,
		PixelFormat formatColor, PixelFormat formatNormal, PixelFormat formatSpecular)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Vertex, Binding::RangeFlag::Constant });
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
		desc.AddRange({ 4, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 1, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 12, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Vertex | Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Texture::Schema textureSchema;
		textureSchema.AddTexture(Data::MaterialPBR::TEX_COLOR_NAME, Resource::Texture::Type::Tex2D, Resource::Texture::Usage::PixelShader);
		textureSchema.AddTexture(Data::MaterialPBR::TEX_NORMAL_NAME, Resource::Texture::Type::Tex2D, Resource::Texture::Usage::PixelShader);
		textureSchema.AddTexture(Data::MaterialPBR::TEX_SPECULAR_NAME, Resource::Texture::Type::Tex2D, Resource::Texture::Usage::PixelShader);
		textureSchema.AddTexture(Data::MaterialPBR::TEX_HEIGHT_NAME, Resource::Texture::Type::Tex2D, Resource::Texture::Usage::PixelShader);
		buildData.TextureLib.Add(Data::MaterialPBR::TEX_SCHEMA_NAME, std::move(textureSchema));

		const auto& schema = buildData.BindingLib.GetSchema(passData->BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"PhongDepthVS", buildData.ShaderCache);
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout = Vertex::GetLayout();
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

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		// Clearing data on first usage
		cl.Open(dev);
		ZE_DRAW_TAG_BEGIN(cl, L"Lambertian Clear", PixelVal::White);

		renderData.Buffers.ClearDSV(cl, ids.DepthStencil, 1.0f, 0);
		renderData.Buffers.ClearRTV(cl, ids.Color, ColorF4());
		renderData.Buffers.ClearRTV(cl, ids.Normal, ColorF4());
		renderData.Buffers.ClearRTV(cl, ids.Specular, ColorF4());

		ZE_DRAW_TAG_END(cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);

		auto group = Data::GetRenderGroup<Data::RenderLambertian>(renderData.Registry);
		const U64 count = group.size();
		if (count)
		{
			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<ModelTransform, ModelTransformBuffer, BUFFER_SHRINK_STEP>(dev, data.TransformBuffers, count);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			const Matrix viewProjection = reinterpret_cast<CameraPBR*>(renderData.DynamicData)->ViewProjection;

			// Depth pre-pass
			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			for (U64 i = 0, j = 0; i < count; ++j)
			{
				cl.Open(dev, data.StateDepth);
				ZE_DRAW_TAG_BEGIN(cl, (L"Lambertian Depth Batch_" + std::to_wstring(j)).c_str(), Pixel(0xC2, 0xC5, 0xCC));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetDSV(cl, ids.DepthStencil);

				ctx.SetFromEnd(2);
				auto& cbuffer = data.TransformBuffers.at(j);
				cbuffer.Bind(cl, ctx);
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
				ZE_DRAW_TAG_BEGIN(cl, (L"Lambertian Batch_" + std::to_wstring(j)).c_str(), Pixel(0xC2, 0xC5, 0xCC));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput<3>(cl, &ids.Color, ids.DepthStencil, true);

				ctx.SetFromEnd(2);
				data.TransformBuffers.at(j).Bind(cl, ctx);
				renderData.DynamicBuffer.Bind(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				for (U32 k = 0; k < TransformBuffer::TRANSFORM_COUNT && i < count; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(k)).c_str(), Pixel(0xAD, 0xAD, 0xC9));
					Resource::Constant<U32> meshBatchId(dev, k);
					meshBatchId.Bind(cl, ctx);

					auto entity = group[i];
					auto material = group.get<Data::MaterialID>(entity);
					if (currentMaterial.ID != material.ID)
					{
						currentMaterial = material;

						const auto& buffers = renderData.Resources.get<Data::MaterialBuffersPBR>(material.ID);
						buffers.BindBuffer(cl, ctx);
						buffers.BindTextures(cl, ctx);
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