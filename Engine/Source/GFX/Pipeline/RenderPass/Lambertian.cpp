#include "GFX/Pipeline/RenderPass/Lambertian.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Vertex.h"

namespace ZE::GFX::Pipeline::RenderPass::Lambertian
{
	void Clean(void* data)
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->StatesSolid.DeleteArray();
		execData->StatesTransparent.DeleteArray();
		delete execData;
	}

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
		psoDesc.RenderTargetsCount = 3;
		psoDesc.FormatsRT[0] = formatColor;
		psoDesc.FormatsRT[1] = formatNormal;
		psoDesc.FormatsRT[2] = formatSpecular;
		const std::wstring shaderName = L"PhongPS";
		U8 stateIndex = Data::MaterialPBR::GetPipelineStateNumber(-1) + 1;
		passData->StatesSolid = new Resource::PipelineStateGfx[stateIndex];
		passData->StatesTransparent = new Resource::PipelineStateGfx[stateIndex];
		while (stateIndex--)
		{
			const wchar_t* suffix = Data::MaterialPBR::DecodeShaderSuffix(Data::MaterialPBR::GetShaderFlagsForState(stateIndex));
			psoDesc.SetShader(psoDesc.PS, (shaderName + suffix).c_str(), buildData.ShaderCache);

			psoDesc.DepthStencil = Resource::DepthStencilMode::DepthBefore;
			ZE_PSO_SET_NAME(psoDesc, "LambertianSolid" + ZE::Utils::ToAscii(suffix));
			passData->StatesSolid[stateIndex].Init(dev, psoDesc, schema);

			psoDesc.DepthStencil = Resource::DepthStencilMode::StencilOff;
			ZE_PSO_SET_NAME(psoDesc, "LambertianTransparent" + ZE::Utils::ToAscii(suffix));
			passData->StatesTransparent[stateIndex].Init(dev, psoDesc, schema);
		}

		passData->TransformBuffers.Exec([&dev](auto& x) { x.emplace_back(dev, nullptr, static_cast<U32>(sizeof(ModelTransformBuffer)), true); });

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		const Resources ids = *passData.Buffers.CastConst<Resources>();
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

		const RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
		const CameraPBR& dynamicData = *reinterpret_cast<CameraPBR*>(renderData.DynamicData);
		const Matrix viewProjection = dynamicData.ViewProjection;
		const Vector cameraPos = Math::XMLoadFloat3(&dynamicData.CameraPos);

		// Compute visibility of objects inside camera view
		Math::BoundingFrustum frustum(Math::XMLoadFloat4x4(&renderer.GetProjection()), false);
		frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&renderer.GetCameraRotation()), cameraPos);
		Utils::FrustumCulling<InsideFrustumSolid, InsideFrustumNotSolid>(renderData.Registry, renderData.Resources,
			Data::GetRenderGroup<Data::RenderLambertian>(renderData.Registry), frustum);

		// Use new group visible only in current frustum and sort
		auto solidGroup = Data::GetVisibleRenderGroup<Data::RenderLambertian, InsideFrustumSolid>(renderData.Registry);
		auto transparentGroup = Data::GetVisibleRenderGroup<Data::RenderLambertian, InsideFrustumNotSolid>(renderData.Registry);
		const U64 solidCount = solidGroup.size();
		const U64 transparentCount = transparentGroup.size();
		Utils::ViewSortAscending(solidGroup, cameraPos);

		// Resize temporary buffer for transform data
		Utils::ResizeTransformBuffers<ModelTransform, ModelTransformBuffer, BUFFER_SHRINK_STEP>(dev, data.TransformBuffers.Get(), solidCount + transparentCount);
		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		// Depth pre-pass
		// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
		for (U64 i = 0, j = 0; i < solidCount; ++j)
		{
			cl.Open(dev, data.StateDepth);
			ZE_DRAW_TAG_BEGIN(cl, (L"Lambertian Depth Batch_" + std::to_wstring(j)).c_str(), Pixel(0xC2, 0xC5, 0xCC));
			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetDSV(cl, ids.DepthStencil);

			ctx.SetFromEnd(2);
			auto& cbuffer = data.TransformBuffers.Get().at(j);
			cbuffer.Bind(cl, ctx);
			ctx.Reset();

			// Compute single batch
			ModelTransformBuffer* buffer = reinterpret_cast<ModelTransformBuffer*>(cbuffer.GetRegion());
			for (U32 k = 0; k < ModelTransformBuffer::TRANSFORM_COUNT && i < solidCount; ++k, ++i)
			{
				ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(k)).c_str(), PixelVal::Gray);

				EID entity = solidGroup[i];
				const auto& transform = solidGroup.get<Data::TransformGlobal>(entity);
				buffer->Transforms[k].Model = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
				buffer->Transforms[k].ModelViewProjection = viewProjection * buffer->Transforms[k].Model;

				Resource::Constant<U32> meshBatchId(dev, k);
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
		}

		// Solid pass
		EID currentMaterial = INVALID_EID;
		U64 currentBuffer = 0;
		U64 currentTransform = 0;
		U8 currentState = -1;
		for (U64 i = 0; i < solidCount; ++currentBuffer)
		{
			cl.Open(dev);
			ZE_DRAW_TAG_BEGIN(cl, (L"Lambertian Solid Batch_" + std::to_wstring(currentBuffer)).c_str(), Pixel(0xC2, 0xC5, 0xCC));
			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetOutput<3>(cl, &ids.Color, ids.DepthStencil, true);

			ctx.SetFromEnd(2);
			data.TransformBuffers.Get().at(currentBuffer).Bind(cl, ctx);
			renderData.DynamicBuffers.Get().Bind(cl, ctx);
			renderData.SettingsBuffer.Bind(cl, ctx);
			ctx.Reset();

			// Compute single batch
			for (currentTransform = 0; currentTransform < TransformBuffer::TRANSFORM_COUNT && i < solidCount; ++currentTransform, ++i)
			{
				ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(currentTransform)).c_str(), Pixel(0xAD, 0xAD, 0xC9));
				Resource::Constant<U32> meshBatchId(dev, currentTransform);
				meshBatchId.Bind(cl, ctx);

				EID entity = solidGroup[i];
				const Data::MaterialID material = solidGroup.get<Data::MaterialID>(entity);
				if (currentMaterial != material.ID)
				{
					currentMaterial = material.ID;

					const auto& buffers = renderData.Resources.get<Data::MaterialBuffersPBR>(currentMaterial);
					buffers.BindBuffer(cl, ctx);
					buffers.BindTextures(cl, ctx);

					const U8 state = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(currentMaterial));
					if (currentState != state)
					{
						currentState = state;
						data.StatesSolid[state].Bind(cl);
					}
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
			currentMaterial = INVALID_EID;
			currentState = -1;
		}

		// Transparent pass
		Utils::ViewSortDescending(transparentGroup, cameraPos);
		currentMaterial = INVALID_EID;
		currentBuffer = solidCount / TransformBuffer::TRANSFORM_COUNT;
		if (currentTransform == TransformBuffer::TRANSFORM_COUNT)
			currentTransform = 0;
		currentState = -1;
		for (U64 i = 0; i < transparentCount; ++currentBuffer)
		{
			cl.Open(dev);
			ZE_DRAW_TAG_BEGIN(cl, (L"Lambertian Transparent Batch_" + std::to_wstring(currentBuffer)).c_str(), Pixel(0xEC, 0xED, 0xEF));
			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetOutput<3>(cl, &ids.Color, ids.DepthStencil, true);

			ctx.SetFromEnd(2);
			auto& cbuffer = data.TransformBuffers.Get().at(currentBuffer);
			cbuffer.Bind(cl, ctx);
			renderData.DynamicBuffers.Get().Bind(cl, ctx);
			renderData.SettingsBuffer.Bind(cl, ctx);
			ctx.Reset();

			// Compute single batch
			ModelTransformBuffer* buffer = reinterpret_cast<ModelTransformBuffer*>(cbuffer.GetRegion());
			for (; currentTransform < TransformBuffer::TRANSFORM_COUNT && i < transparentCount; ++currentTransform, ++i)
			{
				ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(currentTransform)).c_str(), Pixel(0xD6, 0xD6, 0xE4));

				EID entity = transparentGroup[i];
				const auto& transform = transparentGroup.get<Data::TransformGlobal>(entity);
				buffer->Transforms[currentTransform].Model = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
				buffer->Transforms[currentTransform].ModelViewProjection = viewProjection * buffer->Transforms[currentTransform].Model;

				Resource::Constant<U32> meshBatchId(dev, currentTransform);
				meshBatchId.Bind(cl, ctx);

				const Data::MaterialID material = transparentGroup.get<Data::MaterialID>(entity);
				if (currentMaterial != material.ID)
				{
					currentMaterial = material.ID;

					const auto& buffers = renderData.Resources.get<Data::MaterialBuffersPBR>(material.ID);
					buffers.BindBuffer(cl, ctx);
					buffers.BindTextures(cl, ctx);

					const U8 state = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(currentMaterial));
					if (currentState != state)
					{
						currentState = state;
						data.StatesTransparent[state].Bind(cl);
					}
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
			currentMaterial = INVALID_EID;
			currentState = -1;
		}
		// Remove current visibility
		renderData.Registry.clear<InsideFrustumSolid, InsideFrustumNotSolid>();
	}
}