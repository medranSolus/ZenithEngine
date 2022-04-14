#include "GFX/Pipeline/RenderPass/Lambertian.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Pipeline/RendererPBR.h"
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
		desc.AddRange({ 1, 0, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // MaterialPBR buffer
		desc.AddRange({ 4, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Texture, normal, specular, parallax
		desc.AddRange({ 1, 12, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Renderer dynamic data
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

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		auto& cbuffer = renderData.DynamicBuffers.Get();

		EID currentMaterial = INVALID_EID;
		U8 currentState = -1;
		if (solidCount)
		{
			Utils::ViewSortAscending(solidGroup, cameraPos);

			// Depth pre-pass
			cl.Open(dev, data.StateDepth);
			ZE_DRAW_TAG_BEGIN(cl, L"Lambertian Depth", Pixel(0xC2, 0xC5, 0xCC));
			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetDSV(cl, ids.DepthStencil);
			for (U64 i = 0; i < solidCount; ++i)
			{
				ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(i)).c_str(), PixelVal::Gray);

				EID entity = solidGroup[i];
				const auto& transform = solidGroup.get<Data::TransformGlobal>(entity);

				ModelTransformBuffer transformBuffer;
				transformBuffer.Model = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
				transformBuffer.ModelViewProjection = viewProjection * transformBuffer.Model;

				auto& transformInfo = solidGroup.get<InsideFrustumSolid>(entity);
				transformInfo.Transform = cbuffer.Alloc(dev, &transformBuffer, sizeof(ModelTransformBuffer));

				cbuffer.Bind(cl, ctx, transformInfo.Transform);
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

			// Sort by pipeline state
			solidGroup.sort<Data::MaterialID>([&](const auto& m1, const auto& m2) -> bool
				{
					const U8 state1 = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(m1.ID));
					const U8 state2 = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(m2.ID));
					return state1 < state2;
				});
			currentState = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(solidGroup.get<Data::MaterialID>(solidGroup[0]).ID));

			// Solid pass
			cl.Open(dev, data.StatesSolid[currentState]);
			ZE_DRAW_TAG_BEGIN(cl, L"Lambertian Solid", Pixel(0xC2, 0xC5, 0xCC));
			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetOutput<3>(cl, &ids.Color, ids.DepthStencil, true);

			ctx.SetFromEnd(1);
			renderData.BindRendererDynamicData(cl, ctx);
			renderData.SettingsBuffer.Bind(cl, ctx);
			ctx.Reset();
			for (U64 i = 0; i < solidCount; ++i)
			{
				ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(i)).c_str(), Pixel(0xAD, 0xAD, 0xC9));

				EID entity = solidGroup[i];
				cbuffer.Bind(cl, ctx, solidGroup.get<InsideFrustumSolid>(entity).Transform);

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
		if (transparentCount)
		{
			Utils::ViewSortDescending(transparentGroup, cameraPos);

			cl.Open(dev);
			ZE_DRAW_TAG_BEGIN(cl, L"Lambertian Transparent", Pixel(0xEC, 0xED, 0xEF));
			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetOutput<3>(cl, &ids.Color, ids.DepthStencil, true);

			ctx.SetFromEnd(1);
			renderData.BindRendererDynamicData(cl, ctx);
			renderData.SettingsBuffer.Bind(cl, ctx);
			ctx.Reset();
			for (U64 i = 0; i < transparentCount; ++i)
			{
				ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(i)).c_str(), Pixel(0xD6, 0xD6, 0xE4));

				EID entity = transparentGroup[i];
				const auto& transform = transparentGroup.get<Data::TransformGlobal>(entity);

				ModelTransformBuffer transformBuffer;
				transformBuffer.Model = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
				transformBuffer.ModelViewProjection = viewProjection * transformBuffer.Model;
				cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(ModelTransformBuffer));

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
		}
		// Remove current visibility
		renderData.Registry.clear<InsideFrustumSolid, InsideFrustumNotSolid>();
	}
}