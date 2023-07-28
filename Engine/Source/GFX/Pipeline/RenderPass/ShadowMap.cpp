#include "GFX/Pipeline/RenderPass/ShadowMap.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Vertex.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMap
{
	void Clean(Device& dev, ExecuteData& data) noexcept
	{
		data.StateDepth.Free(dev);
		U8 stateCount = Data::MaterialPBR::GetPipelineStateNumber(Data::MaterialPBR::UseTexture | Data::MaterialPBR::UseNormal | Data::MaterialPBR::UseParallax) + 1;
		while (stateCount--)
		{
			data.StatesSolid[stateCount].Free(dev);
			data.StatesTransparent[stateCount].Free(dev);
		}
		data.StatesSolid.DeleteArray();
		data.StatesTransparent.DeleteArray();
	}

	void Setup(Device& dev, RendererBuildData& buildData, ExecuteData& passData,
		PixelFormat formatDS, PixelFormat formatRT, Matrix&& projection)
	{
		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ sizeof(Float4), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light shadow data
		desc.AddRange({ 4, 0, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Texture, normal, specular (not used), parallax
		desc.AddRange({ 1, 12, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer });  // Renderer dynamic data
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData.BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		const auto& schema = buildData.BindingLib.GetSchema(passData.BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "PhongDepthVS", buildData.ShaderCache);
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout = Vertex::GetLayout();
		ZE_PSO_SET_NAME(psoDesc, "ShadowMapDepth");
		passData.StateDepth.Init(dev, psoDesc, schema);

		psoDesc.SetShader(dev, psoDesc.VS, "PhongVS", buildData.ShaderCache);
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		const std::string shaderName = "ShadowPS";
		// Ignore flag UseSpecular as it does not have impact on shadows
		U8 stateIndex = Data::MaterialPBR::GetPipelineStateNumber(Data::MaterialPBR::UseTexture | Data::MaterialPBR::UseNormal | Data::MaterialPBR::UseParallax) + 1;
		passData.StatesSolid = new Resource::PipelineStateGfx[stateIndex];
		passData.StatesTransparent = new Resource::PipelineStateGfx[stateIndex];
		while (stateIndex--)
		{
			const char* suffix = Data::MaterialPBR::DecodeShaderSuffix(Data::MaterialPBR::GetShaderFlagsForState(stateIndex));
			psoDesc.SetShader(dev, psoDesc.PS, (shaderName + suffix).c_str(), buildData.ShaderCache);

			psoDesc.DepthStencil = Resource::DepthStencilMode::DepthBefore;
			ZE_PSO_SET_NAME(psoDesc, "ShadowMapSolid" + std::string(suffix));
			passData.StatesSolid[stateIndex].Init(dev, psoDesc, schema);

			psoDesc.DepthStencil = Resource::DepthStencilMode::StencilOff;
			ZE_PSO_SET_NAME(psoDesc, "ShadowMapTransparent" + std::string(suffix));
			passData.StatesTransparent[stateIndex].Init(dev, psoDesc, schema);
		}

		passData.Projection = std::move(projection);
	}

	Matrix Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData,
		ExecuteData& data, const Resources& ids, const Float3& lightPos,
		const Float3& lightDir, const Math::BoundingFrustum& frustum)
	{
		// Clearing data on first usage
		ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Clear", PixelVal::Gray);
		renderData.Buffers.ClearDSV(cl, ids.Depth, 1.0f, 0);
		renderData.Buffers.ClearRTV(cl, ids.RenderTarget, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
		ZE_DRAW_TAG_END(dev, cl);

		// Prepare view-projection for shadow
		const Vector position = Math::XMLoadFloat3(&lightPos);
		const Vector direction = Math::XMLoadFloat3(&lightDir);
		Matrix viewProjection = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position, direction, Math::XMVector3Orthogonal(direction)) * data.Projection);

		auto group = Data::GetRenderGroup<Data::ShadowCaster>(renderData.Registry);
		if (group.size())
		{
			// Compute visibility of objects inside camera view
			Utils::FrustumCulling<InsideFrustumSolid, InsideFrustumNotSolid>(renderData.Registry, renderData.Resources, group, frustum);

			// Use new group visible only in current frustum and sort
			auto solidGroup = Data::GetVisibleRenderGroup<Data::ShadowCaster, InsideFrustumSolid>(renderData.Registry);
			auto transparentGroup = Data::GetVisibleRenderGroup<Data::ShadowCaster, InsideFrustumNotSolid>(renderData.Registry);
			const U64 solidCount = solidGroup.size();
			const U64 transparentCount = transparentGroup.size();

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			auto& cbuffer = renderData.DynamicBuffers.Get();

			EID currentMaterial = INVALID_EID;
			U8 currentState = UINT8_MAX;
			Resource::Constant<Float4> shadowData(dev, Float4(lightPos.x, lightPos.y, lightPos.z, 0.0f));
			if (solidCount)
			{
				Utils::ViewSortAscending(solidGroup, position);

				// Depth pre-pass
				data.StateDepth.Bind(cl);
				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Depth", Pixel(0x98, 0x9F, 0xA7));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetDSV(cl, ids.Depth);

				ctx.SetFromEnd(1);
				renderData.BindRendererDynamicData(cl, ctx);
				ctx.Reset();
				for (U64 i = 0; i < solidCount; ++i)
				{
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), PixelVal::Gray);

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
					ZE_DRAW_TAG_END(dev, cl);
				}
				ZE_DRAW_TAG_END(dev, cl);

				// Sort by pipeline state
				solidGroup.sort<Data::MaterialID>([&](const auto& m1, const auto& m2) -> bool
					{
						const U8 state1 = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(m1.ID) & ~Data::MaterialPBR::UseSpecular);
						const U8 state2 = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(m2.ID) & ~Data::MaterialPBR::UseSpecular);
						return state1 < state2;
					});
				currentState = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(solidGroup.get<Data::MaterialID>(solidGroup[0]).ID) & ~Data::MaterialPBR::UseSpecular);

				// Solid pass
				data.StatesSolid[currentState].Bind(cl);
				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Solid", Pixel(0x79, 0x82, 0x8D));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(1);
				renderData.BindRendererDynamicData(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();
				for (U64 i = 0; i < solidCount; ++i)
				{
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0x5D, 0x5E, 0x61));

					EID entity = solidGroup[i];
					cbuffer.Bind(cl, ctx, solidGroup.get<InsideFrustumSolid>(entity).Transform);

					const Data::MaterialID material = solidGroup.get<Data::MaterialID>(entity);
					if (currentMaterial != material.ID)
					{
						currentMaterial = material.ID;

						const auto& matData = renderData.Resources.get<Data::MaterialPBR>(currentMaterial);
						shadowData.Set(dev, Float4(lightPos.x, lightPos.y, lightPos.z, matData.ParallaxScale));
						shadowData.Bind(cl, ctx);
						renderData.Resources.get<Data::MaterialBuffersPBR>(currentMaterial).BindTextures(cl, ctx);

						const U8 state = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(currentMaterial) & ~Data::MaterialPBR::UseSpecular);
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
					ZE_DRAW_TAG_END(dev, cl);
				}
				ZE_DRAW_TAG_END(dev, cl);
				currentMaterial = INVALID_EID;
				currentState = UINT8_MAX;
			}

			// Transparent pass
			if (transparentCount)
			{
				Utils::ViewSortDescending(transparentGroup, position);

				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Transparent", Pixel(0x79, 0x82, 0x8D));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(1);
				renderData.BindRendererDynamicData(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();
				for (U64 i = 0; i < transparentCount; ++i)
				{
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0x5D, 0x5E, 0x61));

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

						const auto& matData = renderData.Resources.get<Data::MaterialPBR>(material.ID);
						shadowData.Set(dev, Float4(lightPos.x, lightPos.y, lightPos.z, matData.ParallaxScale));
						shadowData.Bind(cl, ctx);
						renderData.Resources.get<Data::MaterialBuffersPBR>(material.ID).BindTextures(cl, ctx);

						const U8 state = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(currentMaterial) & ~Data::MaterialPBR::UseSpecular);
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
					ZE_DRAW_TAG_END(dev, cl);
				}
				ZE_DRAW_TAG_END(dev, cl);
			}
			// Remove current visibility indication
			renderData.Registry.clear<InsideFrustumSolid, InsideFrustumNotSolid>();
		}
		return viewProjection;
	}
}