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
		desc.AddRange({ 1, 0, 4, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ sizeof(Float4), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light shadow data
		desc.AddRange({ 4, 0, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Texture, normal, specular (not used), parallax
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Geometry, Binding::RangeFlag::CBV }); // Cube view buffer
		desc.AddRange({ 1, 12, 1, Resource::ShaderType::Geometry, Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer }); // Renderer dynamic data
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
			ZE_PERF_GUARD("Shadow Map Cube - present");
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
			auto& cbuffer = *renderData.DynamicBuffer;
			auto cubeBufferInfo = cbuffer.Alloc(dev, &viewBuffer, sizeof(CubeViewBuffer));

			// Split into groups based on materials and if inside light volume
			ZE_PERF_START("Shadow Map Cube - visibility group split loop");
			const Math::BoundingSphere lightSphere(lightPos, lightVolume);
			for (EID entity : group)
			{
				ZE_PERF_GUARD("Shadow Map Cube - visibility group split single loop item");
				const auto& transform = group.get<Data::TransformGlobal>(entity);

				Math::BoundingBox box = renderData.Assets.GetResources().get<Math::BoundingBox>(group.get<Data::MeshID>(entity).ID);
				box.Transform(box, Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

				if (box.Intersects(lightSphere))
				{
					if (renderData.Assets.GetResources().all_of<Data::MaterialNotSolid>(group.get<Data::MaterialID>(entity).ID))
						renderData.Registry.emplace<Transparent>(entity);
					else
						renderData.Registry.emplace<Solid>(entity);
				}
			}
			ZE_PERF_STOP();

			auto solidGroup = Data::GetVisibleRenderGroup<Data::ShadowCaster, Solid>(renderData.Registry);
			auto transparentGroup = Data::GetVisibleRenderGroup<Data::ShadowCaster, Transparent>(renderData.Registry);
			const U64 solidCount = solidGroup.size();
			const U64 transparentCount = transparentGroup.size();

			EID currentMaterial = INVALID_EID;
			U8 currentState = UINT8_MAX;
			Resource::Constant<Float4> shadowData(dev, Float4(lightPos.x, lightPos.y, lightPos.z, 0.0f));
			if (solidCount)
			{
				ZE_PERF_GUARD("Shadow Map Cube - solid present");

				ZE_PERF_START("Shadow Map Cube - solid view sort");
				Utils::ViewSortAscending(solidGroup, position);
				ZE_PERF_STOP();

				// Depth pre-pass
				ZE_PERF_START("Shadow Map Cube Depth");
				data.StateDepth.Bind(cl);
				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Cube Depth", Pixel(0x75, 0x7C, 0x88));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetDSV(cl, ids.Depth);

				ctx.SetFromEnd(2);
				cbuffer.Bind(cl, ctx, cubeBufferInfo);
				renderData.BindRendererDynamicData(cl, ctx);
				ctx.Reset();

				ZE_PERF_START("Shadow Map Cube Depth - main loop");
				for (U64 i = 0; i < solidCount; ++i)
				{
					ZE_PERF_GUARD("Shadow Map Cube Depth - single loop item");
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), PixelVal::Gray);

					EID entity = solidGroup[i];
					const auto& transform = solidGroup.get<Data::TransformGlobal>(entity);

					TransformBuffer transformBuffer;
					transformBuffer.Transform = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

					auto& transformInfo = solidGroup.get<Solid>(entity);
					transformInfo.Transform = cbuffer.Alloc(dev, &transformBuffer, sizeof(TransformBuffer));
					cbuffer.Bind(cl, ctx, transformInfo.Transform);
					ctx.Reset();

					renderData.Assets.GetResources().get<Resource::Mesh>(solidGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
					ZE_DRAW_TAG_END(dev, cl);
				}
				ZE_PERF_STOP();

				ZE_DRAW_TAG_END(dev, cl);
				ZE_PERF_STOP();

				// Sort by pipeline state
				ZE_PERF_START("Shadow Map Cube - solid material sort");
				solidGroup.sort<Data::MaterialID>([&](const auto& m1, const auto& m2) -> bool
					{
						const U8 state1 = Data::MaterialPBR::GetPipelineStateNumber(renderData.Assets.GetResources().get<Data::PBRFlags>(m1.ID) & ~Data::MaterialPBR::UseSpecular);
						const U8 state2 = Data::MaterialPBR::GetPipelineStateNumber(renderData.Assets.GetResources().get<Data::PBRFlags>(m2.ID) & ~Data::MaterialPBR::UseSpecular);
						return state1 < state2;
					});
				currentState = Data::MaterialPBR::GetPipelineStateNumber(renderData.Assets.GetResources().get<Data::PBRFlags>(solidGroup.get<Data::MaterialID>(solidGroup[0]).ID) & ~Data::MaterialPBR::UseSpecular);
				ZE_PERF_STOP();

				// Solid pass
				ZE_PERF_START("Shadow Map Cube Solid");
				data.StatesSolid[currentState].Bind(cl);
				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Cube Solid", Pixel(0x52, 0xB2, 0xBF));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(2);
				cbuffer.Bind(cl, ctx, cubeBufferInfo);
				renderData.BindRendererDynamicData(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				ZE_PERF_START("Shadow Map Cube Solid - main loop");
				for (U64 i = 0; i < solidCount; ++i)
				{
					ZE_PERF_GUARD("Shadow Map Cube Solid - single loop item");
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0x01, 0x60, 0x64));

					EID entity = solidGroup[i];
					cbuffer.Bind(cl, ctx, solidGroup.get<Solid>(entity).Transform);

					const Data::MaterialID material = solidGroup.get<Data::MaterialID>(entity);
					if (currentMaterial != material.ID)
					{
						currentMaterial = material.ID;

						const auto& matData = renderData.Assets.GetResources().get<Data::MaterialPBR>(currentMaterial);
						shadowData.Set(dev, Float4(lightPos.x, lightPos.y, lightPos.z, matData.ParallaxScale));
						shadowData.Bind(cl, ctx);
						renderData.Assets.GetResources().get<Data::MaterialBuffersPBR>(currentMaterial).BindTextures(cl, ctx);

						const U8 state = Data::MaterialPBR::GetPipelineStateNumber(renderData.Assets.GetResources().get<Data::PBRFlags>(currentMaterial) & ~Data::MaterialPBR::UseSpecular);
						if (currentState != state)
						{
							currentState = state;
							data.StatesSolid[state].Bind(cl);
						}
					}
					ctx.Reset();

					renderData.Assets.GetResources().get<Resource::Mesh>(solidGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
					ZE_DRAW_TAG_END(dev, cl);
				}
				ZE_PERF_STOP();

				ZE_DRAW_TAG_END(dev, cl);
				ZE_PERF_STOP();

				currentMaterial = INVALID_EID;
				currentState = UINT8_MAX;
			}

			// Transparent pass
			if (transparentCount)
			{
				ZE_PERF_GUARD("Shadow Map Cube - transparent present");

				ZE_PERF_START("Shadow Map Cube - transparent view sort");
				Utils::ViewSortDescending(transparentGroup, position);
				ZE_PERF_STOP();

				ZE_PERF_START("Shadow Map Cube Transparent");
				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Cube Transparent", Pixel(0x52, 0xB2, 0xBF));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.Depth);

				ctx.SetFromEnd(2);
				cbuffer.Bind(cl, ctx, cubeBufferInfo);
				renderData.BindRendererDynamicData(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				ZE_PERF_START("Shadow Map Cube Transparent - main loop");
				for (U64 i = 0; i < transparentCount; ++i)
				{
					ZE_PERF_GUARD("Shadow Map Cube Transparent - single loop item");
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

						const auto& matData = renderData.Assets.GetResources().get<Data::MaterialPBR>(material.ID);
						shadowData.Set(dev, Float4(lightPos.x, lightPos.y, lightPos.z, matData.ParallaxScale));
						shadowData.Bind(cl, ctx);
						renderData.Assets.GetResources().get<Data::MaterialBuffersPBR>(material.ID).BindTextures(cl, ctx);

						const U8 state = Data::MaterialPBR::GetPipelineStateNumber(renderData.Assets.GetResources().get<Data::PBRFlags>(currentMaterial) & ~Data::MaterialPBR::UseSpecular);
						if (currentState != state)
						{
							currentState = state;
							data.StatesTransparent[state].Bind(cl);
						}
					}
					ctx.Reset();

					renderData.Assets.GetResources().get<Resource::Mesh>(transparentGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
					ZE_DRAW_TAG_END(dev, cl);
				}
				ZE_PERF_STOP();

				ZE_DRAW_TAG_END(dev, cl);
				ZE_PERF_STOP();
			}
			// Remove current material indication
			ZE_PERF_START("Shadow Map Cube - visibility clear");
			renderData.Registry.clear<Solid, Transparent>();
			ZE_PERF_STOP();
		}
	}
}