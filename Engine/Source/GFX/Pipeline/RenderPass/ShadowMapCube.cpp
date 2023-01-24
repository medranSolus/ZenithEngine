#include "GFX/Pipeline/RenderPass/ShadowMapCube.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Vertex.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMapCube
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

	void Setup(Device& dev, RendererBuildData& buildData, ExecuteData& passData, PixelFormat formatDS, PixelFormat formatRT)
	{
		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ sizeof(float), 1, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Parallax scale
		desc.AddRange({ 4, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });  // Texture, normal, specular (not used), parallax
		desc.AddRange({ 1, 0, Resource::ShaderType::Geometry, Binding::RangeFlag::CBV }); // Cube view buffer
		desc.AddRange({ 1, 12, Resource::ShaderType::Geometry, Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer }); // Renderer dynamic data
		desc.AddRange({ sizeof(Float3), 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light position
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData.BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		const auto& schema = buildData.BindingLib.GetSchema(passData.BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "ShadowCubeDepthVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.GS, "ShadowCubeDepthGS", buildData.ShaderCache);
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout = Vertex::GetLayout();
		ZE_PSO_SET_NAME(psoDesc, "ShadowMapCubeDepth");
		passData.StateDepth.Init(dev, psoDesc, schema);

		psoDesc.SetShader(dev, psoDesc.VS, "ShadowCubeVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.GS, "ShadowCubeGS", buildData.ShaderCache);
		psoDesc.RenderTargetsCount = 6;
		for (U8 i = 0; i < psoDesc.RenderTargetsCount; ++i)
			psoDesc.FormatsRT[i] = formatRT;
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
			ZE_PSO_SET_NAME(psoDesc, "ShadowMapCubeSolid" + std::string(suffix));
			passData.StatesSolid[stateIndex].Init(dev, psoDesc, schema);

			psoDesc.DepthStencil = Resource::DepthStencilMode::StencilOff;
			ZE_PSO_SET_NAME(psoDesc, "ShadowMapCubeTransparent" + std::string(suffix));
			passData.StatesTransparent[stateIndex].Init(dev, psoDesc, schema);
		}

		passData.Projection = Math::XMMatrixPerspectiveFovLH(static_cast<float>(M_PI_2), 1.0f, 0.01f, 1000.0f);
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData,
		ExecuteData& data, const Resources& ids, const Float3& lightPos, float lightVolume)
	{
		// Clearing data on first usage
		ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Cube Clear", PixelVal::Gray);
		renderData.Buffers.ClearDSV(cl, ids.Depth, 1.0f, 0);
		renderData.Buffers.ClearRTV(cl, ids.RenderTarget, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
		ZE_DRAW_TAG_END(dev, cl);

		auto group = Data::GetRenderGroup<Data::ShadowCaster>(renderData.Registry);
		if (group.size())
		{
			// Prepare view-projections for casting onto 6 faces
			CubeViewBuffer viewBuffer;
			const Vector position = Math::XMLoadFloat3(&lightPos);
			const Vector up = { 0.0f, 1.0f, 0.0f, 0.0f };
			// +x
			viewBuffer.ViewProjection[0] = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 1.0f, 0.0f, 0.0f, 0.0f }, up) * data.Projection);
			// -x
			viewBuffer.ViewProjection[1] = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ -1.0f, 0.0f, 0.0f, 0.0f }, up) * data.Projection);
			// +y
			viewBuffer.ViewProjection[2] = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }) * data.Projection);
			// -y
			viewBuffer.ViewProjection[3] = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 0.0f, -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }) * data.Projection);
			// +z
			viewBuffer.ViewProjection[4] = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 0.0f, 0.0f, 1.0f, 0.0f }, up) * data.Projection);
			// -z
			viewBuffer.ViewProjection[5] = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 0.0f, 0.0f, -1.0f, 0.0f }, up) * data.Projection);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			auto& cbuffer = renderData.DynamicBuffers.Get();
			auto cubeBufferInfo = cbuffer.Alloc(dev, &viewBuffer, sizeof(CubeViewBuffer));

			// Split into groups based on materials and if inside light volume
			const Math::BoundingSphere lightSphere(lightPos, lightVolume);
			for (EID entity : group)
			{
				const auto& transform = group.get<Data::TransformGlobal>(entity);

				Math::BoundingBox box = renderData.Resources.get<Math::BoundingBox>(group.get<Data::MeshID>(entity).ID);
				box.Transform(box, Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

				if (box.Intersects(lightSphere))
				{
					if (renderData.Resources.all_of<Data::MaterialNotSolid>(group.get<Data::MaterialID>(entity).ID))
						renderData.Registry.emplace<Transparent>(entity);
					else
						renderData.Registry.emplace<Solid>(entity);
				}
			}
			auto solidGroup = Data::GetVisibleRenderGroup<Data::ShadowCaster, Solid>(renderData.Registry);
			auto transparentGroup = Data::GetVisibleRenderGroup<Data::ShadowCaster, Transparent>(renderData.Registry);
			const U64 solidCount = solidGroup.size();
			const U64 transparentCount = transparentGroup.size();

			EID currentMaterial = INVALID_EID;
			U8 currentState = -1;
			Resource::Constant<Float3> pos(dev, lightPos);
			if (solidCount)
			{
				Utils::ViewSortAscending(solidGroup, position);

				// Depth pre-pass
				data.StateDepth.Bind(cl);
				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Cube Depth", Pixel(0x75, 0x7C, 0x88));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetDSV(cl, ids.Depth);

				ctx.SetFromEnd(3);
				cbuffer.Bind(cl, ctx, cubeBufferInfo);
				renderData.BindRendererDynamicData(cl, ctx);
				ctx.Reset();
				for (U64 i = 0; i < solidCount; ++i)
				{
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), PixelVal::Gray);

					EID entity = solidGroup[i];
					const auto& transform = solidGroup.get<Data::TransformGlobal>(entity);

					TransformBuffer transformBuffer;
					transformBuffer.Transform = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

					auto& transformInfo = solidGroup.get<Solid>(entity);
					transformInfo.Transform = cbuffer.Alloc(dev, &transformBuffer, sizeof(TransformBuffer));
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
				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Cube Solid", Pixel(0x52, 0xB2, 0xBF));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(3);
				cbuffer.Bind(cl, ctx, cubeBufferInfo);
				renderData.BindRendererDynamicData(cl, ctx);
				pos.Bind(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();
				for (U64 i = 0; i < solidCount; ++i)
				{
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0x01, 0x60, 0x64));

					EID entity = solidGroup[i];
					cbuffer.Bind(cl, ctx, solidGroup.get<Solid>(entity).Transform);

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
					ZE_DRAW_TAG_END(dev, cl);
				}
				ZE_DRAW_TAG_END(dev, cl);
				currentMaterial = INVALID_EID;
				currentState = -1;
			}

			// Transparent pass
			if (transparentCount)
			{
				Utils::ViewSortDescending(transparentGroup, position);

				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Cube Transparent", Pixel(0x52, 0xB2, 0xBF));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(3);
				cbuffer.Bind(cl, ctx, cubeBufferInfo);
				renderData.BindRendererDynamicData(cl, ctx);
				pos.Bind(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();
				for (U64 i = 0; i < transparentCount; ++i)
				{
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0x01, 0x60, 0x64));

					EID entity = transparentGroup[i];
					const auto& transform = transparentGroup.get<Data::TransformGlobal>(entity);

					TransformBuffer transformBuffer;
					transformBuffer.Transform = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
					cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(TransformBuffer));

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
					ZE_DRAW_TAG_END(dev, cl);
				}
				ZE_DRAW_TAG_END(dev, cl);
			}
			// Remove current material indication
			renderData.Registry.clear<Solid, Transparent>();
		}
	}
}