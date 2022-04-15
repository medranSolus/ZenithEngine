#include "GFX/Pipeline/RenderPass/ShadowMap.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Vertex.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMap
{
	void Clean(ExecuteData& data)
	{
		data.StatesSolid.DeleteArray();
		data.StatesTransparent.DeleteArray();
	}

	void Setup(Device& dev, RendererBuildData& buildData, ExecuteData& passData,
		PixelFormat formatDS, PixelFormat formatRT, Matrix&& projection)
	{
		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ sizeof(float), 1, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Parallax scale
		desc.AddRange({ 4, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Texture, normal, specular (not used), parallax
		desc.AddRange({ 1, 12, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer });  // Renderer dynamic data
		desc.AddRange({ sizeof(Float3), 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light position
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
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		const std::wstring shaderName = L"ShadowPS";
		// Ignore flag UseSpecular as it does not have impact on shadows
		U8 stateIndex = Data::MaterialPBR::GetPipelineStateNumber(Data::MaterialPBR::UseTexture | Data::MaterialPBR::UseNormal | Data::MaterialPBR::UseParallax) + 1;
		passData.StatesSolid = new Resource::PipelineStateGfx[stateIndex];
		passData.StatesTransparent = new Resource::PipelineStateGfx[stateIndex];
		while (stateIndex--)
		{
			const wchar_t* suffix = Data::MaterialPBR::DecodeShaderSuffix(Data::MaterialPBR::GetShaderFlagsForState(stateIndex));
			psoDesc.SetShader(psoDesc.PS, (shaderName + suffix).c_str(), buildData.ShaderCache);

			psoDesc.DepthStencil = Resource::DepthStencilMode::DepthBefore;
			ZE_PSO_SET_NAME(psoDesc, "ShadowMapSolid" + ZE::Utils::ToAscii(suffix));
			passData.StatesSolid[stateIndex].Init(dev, psoDesc, schema);

			psoDesc.DepthStencil = Resource::DepthStencilMode::StencilOff;
			ZE_PSO_SET_NAME(psoDesc, "ShadowMapTransparent" + ZE::Utils::ToAscii(suffix));
			passData.StatesTransparent[stateIndex].Init(dev, psoDesc, schema);
		}

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
			auto solidGroup = Data::GetVisibleRenderGroup<Data::ShadowCaster, InsideFrustumSolid>(renderData.Registry);
			auto transparentGroup = Data::GetVisibleRenderGroup<Data::ShadowCaster, InsideFrustumNotSolid>(renderData.Registry);
			const U64 solidCount = solidGroup.size();
			const U64 transparentCount = transparentGroup.size();

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			auto& cbuffer = renderData.DynamicBuffers.Get();

			EID currentMaterial = INVALID_EID;
			U8 currentState = -1;
			Resource::Constant<Float3> pos(dev, lightPos);
			if (solidCount)
			{
				Utils::ViewSortAscending(solidGroup, position);

				// Depth pre-pass
				cl.Open(dev, data.StateDepth);
				ZE_DRAW_TAG_BEGIN(cl, L"Shadow Map Depth", Pixel(0x98, 0x9F, 0xA7));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetDSV(cl, ids.Depth);

				ctx.SetFromEnd(2);
				renderData.BindRendererDynamicData(cl, ctx);
				ctx.Reset();
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
						const U8 state1 = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(m1.ID) & ~Data::MaterialPBR::UseSpecular);
						const U8 state2 = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(m2.ID) & ~Data::MaterialPBR::UseSpecular);
						return state1 < state2;
					});
				currentState = Data::MaterialPBR::GetPipelineStateNumber(renderData.Resources.get<Data::PBRFlags>(solidGroup.get<Data::MaterialID>(solidGroup[0]).ID) & ~Data::MaterialPBR::UseSpecular);

				// Solid pass
				cl.Open(dev, data.StatesSolid[currentState]);
				ZE_DRAW_TAG_BEGIN(cl, L"Shadow Map Solid", Pixel(0x79, 0x82, 0x8D));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(2);
				renderData.BindRendererDynamicData(cl, ctx);
				pos.Bind(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();
				for (U64 i = 0; i < solidCount; ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(i)).c_str(), Pixel(0x5D, 0x5E, 0x61));

					EID entity = solidGroup[i];
					cbuffer.Bind(cl, ctx, solidGroup.get<InsideFrustumSolid>(entity).Transform);

					const Data::MaterialID material = solidGroup.get<Data::MaterialID>(entity);
					if (currentMaterial != material.ID)
					{
						currentMaterial = material.ID;

						const auto& matData = renderData.Resources.get<Data::MaterialPBR>(currentMaterial);
						Resource::Constant<float> parallaxScale(dev, matData.ParallaxScale);
						parallaxScale.Bind(cl, ctx);
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
				Utils::ViewSortDescending(transparentGroup, position);

				cl.Open(dev);
				ZE_DRAW_TAG_BEGIN(cl, L"Shadow Map Transparent", Pixel(0x79, 0x82, 0x8D));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(2);
				renderData.BindRendererDynamicData(cl, ctx);
				pos.Bind(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();
				for (U64 i = 0; i < transparentCount; ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(i)).c_str(), Pixel(0x5D, 0x5E, 0x61));

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
						Resource::Constant<float> parallaxScale(dev, matData.ParallaxScale);
						parallaxScale.Bind(cl, ctx);
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
					ZE_DRAW_TAG_END(cl);
				}
				ZE_DRAW_TAG_END(cl);
				cl.Close(dev);
				dev.ExecuteMain(cl);
			}
			// Remove current visibility indication
			renderData.Registry.clear<InsideFrustumSolid, InsideFrustumNotSolid>();
		}
		return viewProjection;
	}
}