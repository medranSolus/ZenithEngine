#include "GFX/Pipeline/RenderPass/Lambertian.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/Vertex.h"

namespace ZE::GFX::Pipeline::RenderPass::Lambertian
{
	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->StateDepth.Free(dev);
		U8 stateCount = Data::MaterialPBR::GetPipelineStateNumber(UINT8_MAX) + 1;
		while (stateCount--)
		{
			execData->StatesSolid[stateCount].Free(dev);
			execData->StatesTransparent[stateCount].Free(dev);
		}
		execData->StatesSolid.DeleteArray();
		execData->StatesTransparent.DeleteArray();
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatDS,
		PixelFormat formatColor, PixelFormat formatNormal, PixelFormat formatSpecular)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ 1, 0, 4, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // MaterialPBR buffer
		desc.AddRange({ 4, 0, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Texture, normal, specular, parallax
		desc.AddRange({ 1, 12, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer }); // Renderer dynamic data
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
		psoDesc.SetShader(dev, psoDesc.VS, "PhongDepthVS", buildData.ShaderCache);
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout = Vertex::GetLayout();
		ZE_PSO_SET_NAME(psoDesc, "LambertianDepth");
		passData->StateDepth.Init(dev, psoDesc, schema);

		psoDesc.SetShader(dev, psoDesc.VS, "PhongVS", buildData.ShaderCache);
		psoDesc.RenderTargetsCount = 3;
		psoDesc.FormatsRT[0] = formatColor;
		psoDesc.FormatsRT[1] = formatNormal;
		psoDesc.FormatsRT[2] = formatSpecular;
		const std::string shaderName = "PhongPS";
		U8 stateIndex = Data::MaterialPBR::GetPipelineStateNumber(UINT8_MAX) + 1;
		passData->StatesSolid = new Resource::PipelineStateGfx[stateIndex];
		passData->StatesTransparent = new Resource::PipelineStateGfx[stateIndex];
		while (stateIndex--)
		{
			const char* suffix = Data::MaterialPBR::DecodeShaderSuffix(Data::MaterialPBR::GetShaderFlagsForState(stateIndex));
			psoDesc.SetShader(dev, psoDesc.PS, (shaderName + suffix).c_str(), buildData.ShaderCache);

			psoDesc.DepthStencil = Resource::DepthStencilMode::DepthBefore;
			ZE_PSO_SET_NAME(psoDesc, "LambertianSolid" + std::string(suffix));
			passData->StatesSolid[stateIndex].Init(dev, psoDesc, schema);

			psoDesc.DepthStencil = Resource::DepthStencilMode::StencilOff;
			ZE_PSO_SET_NAME(psoDesc, "LambertianTransparent" + std::string(suffix));
			passData->StatesTransparent[stateIndex].Init(dev, psoDesc, schema);
		}

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_START("Lambertian");
		const Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		// Clearing data on first usage
		cl.Open(dev);
		ZE_DRAW_TAG_BEGIN(dev, cl, "Lambertian Clear", PixelVal::White);
		renderData.Buffers.ClearDSV(cl, ids.DepthStencil, 1.0f, 0);
		renderData.Buffers.ClearRTV(cl, ids.Color, ColorF4());
		renderData.Buffers.ClearRTV(cl, ids.Normal, ColorF4());
		renderData.Buffers.ClearRTV(cl, ids.Specular, ColorF4());
		ZE_DRAW_TAG_END(dev, cl);

		const RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
		const CameraPBR& dynamicData = *reinterpret_cast<CameraPBR*>(renderData.DynamicData);
		const Matrix viewProjection = dynamicData.ViewProjection;
		const Vector cameraPos = Math::XMLoadFloat3(&dynamicData.CameraPos);

		// Compute visibility of objects inside camera view
		ZE_PERF_START("Lambertian - frustum culling");
		Math::BoundingFrustum frustum(Math::XMLoadFloat4x4(&renderer.GetProjection()), false);
		frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&renderer.GetCameraRotation()), cameraPos);
		Utils::FrustumCulling<InsideFrustumSolid, InsideFrustumNotSolid>(renderData.Registry, renderData.Assets.GetResources(),
			Data::GetRenderGroup<Data::RenderLambertian>(renderData.Registry), frustum);
		ZE_PERF_STOP();

		// Use new group visible only in current frustum and sort
		auto solidGroup = Data::GetVisibleRenderGroup<Data::RenderLambertian, InsideFrustumSolid>(renderData.Registry);
		auto transparentGroup = Data::GetVisibleRenderGroup<Data::RenderLambertian, InsideFrustumNotSolid>(renderData.Registry);
		const U64 solidCount = solidGroup.size();
		const U64 transparentCount = transparentGroup.size();

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		auto& cbuffer = renderData.DynamicBuffers.Get();

		EID currentMaterial = INVALID_EID;
		U8 currentState = UINT8_MAX;
		if (solidCount)
		{
			ZE_PERF_START("Lambertian - solid present");

			ZE_PERF_START("Lambertian - solid view sorting");
			Utils::ViewSortAscending(solidGroup, cameraPos);
			ZE_PERF_STOP();

			// Depth pre-pass
			ZE_PERF_START("Lambertian Depth");
			data.StateDepth.Bind(cl);
			ZE_DRAW_TAG_BEGIN(dev, cl, "Lambertian Depth", Pixel(0xC2, 0xC5, 0xCC));
			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetDSV(cl, ids.DepthStencil);

			ZE_PERF_START("Lambertian Depth - main loop");
			for (U64 i = 0; i < solidCount; ++i)
			{
				ZE_PERF_START("Lambertian Depth - single loop item");
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

				renderData.Assets.GetResources().get<Resource::Mesh>(solidGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
				ZE_DRAW_TAG_END(dev, cl);
				ZE_PERF_STOP();
			}
			ZE_PERF_STOP();

			ZE_DRAW_TAG_END(dev, cl);
			ZE_PERF_STOP();

			// Sort by pipeline state
			ZE_PERF_START("Lambertian - solid material sort");
			solidGroup.sort<Data::MaterialID>([&](const auto& m1, const auto& m2) -> bool
				{
					const U8 state1 = Data::MaterialPBR::GetPipelineStateNumber(renderData.Assets.GetResources().get<Data::PBRFlags>(m1.ID));
					const U8 state2 = Data::MaterialPBR::GetPipelineStateNumber(renderData.Assets.GetResources().get<Data::PBRFlags>(m2.ID));
					return state1 < state2;
				});
			currentState = Data::MaterialPBR::GetPipelineStateNumber(renderData.Assets.GetResources().get<Data::PBRFlags>(solidGroup.get<Data::MaterialID>(solidGroup[0]).ID));
			ZE_PERF_STOP();

			// Solid pass
			ZE_PERF_START("Lambertian Solid");
			data.StatesSolid[currentState].Bind(cl);
			ZE_DRAW_TAG_BEGIN(dev, cl, "Lambertian Solid", Pixel(0xC2, 0xC5, 0xCC));
			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetOutput<3>(cl, &ids.Color, ids.DepthStencil, true);

			ctx.SetFromEnd(1);
			renderData.BindRendererDynamicData(cl, ctx);
			renderData.SettingsBuffer.Bind(cl, ctx);
			ctx.Reset();

			ZE_PERF_START("Lambertian Solid - main loop");
			for (U64 i = 0; i < solidCount; ++i)
			{
				ZE_PERF_START("Lambertian Solid - single loop item");
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0xAD, 0xAD, 0xC9));

				EID entity = solidGroup[i];
				cbuffer.Bind(cl, ctx, solidGroup.get<InsideFrustumSolid>(entity).Transform);

				const Data::MaterialID material = solidGroup.get<Data::MaterialID>(entity);
				if (currentMaterial != material.ID)
				{
					currentMaterial = material.ID;

					const auto& buffers = renderData.Assets.GetResources().get<Data::MaterialBuffersPBR>(currentMaterial);
					buffers.BindBuffer(cl, ctx);
					buffers.BindTextures(cl, ctx);

					const U8 state = Data::MaterialPBR::GetPipelineStateNumber(renderData.Assets.GetResources().get<Data::PBRFlags>(currentMaterial));
					if (currentState != state)
					{
						currentState = state;
						data.StatesSolid[state].Bind(cl);
					}
				}
				ctx.Reset();

				renderData.Assets.GetResources().get<Resource::Mesh>(solidGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
				ZE_DRAW_TAG_END(dev, cl);
				ZE_PERF_STOP();
			}
			ZE_PERF_STOP();

			ZE_DRAW_TAG_END(dev, cl);
			ZE_PERF_STOP();

			currentMaterial = INVALID_EID;
			currentState = UINT8_MAX;
			ZE_PERF_STOP();
		}

		// Transparent pass
		if (transparentCount)
		{
			ZE_PERF_START("Lambertian - transparent present");

			ZE_PERF_START("Lambertian - transparent view sorting");
			Utils::ViewSortDescending(transparentGroup, cameraPos);
			ZE_PERF_STOP();

			ZE_PERF_START("Lambertian Transparent");
			ZE_DRAW_TAG_BEGIN(dev, cl, "Lambertian Transparent", Pixel(0xEC, 0xED, 0xEF));
			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetOutput<3>(cl, &ids.Color, ids.DepthStencil, true);

			ctx.SetFromEnd(1);
			renderData.BindRendererDynamicData(cl, ctx);
			renderData.SettingsBuffer.Bind(cl, ctx);
			ctx.Reset();

			ZE_PERF_START("Lambertian Transparent - main loop");
			for (U64 i = 0; i < transparentCount; ++i)
			{
				ZE_PERF_START("Lambertian Transparent - single loop item");
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0xD6, 0xD6, 0xE4));

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

					const auto& buffers = renderData.Assets.GetResources().get<Data::MaterialBuffersPBR>(material.ID);
					buffers.BindBuffer(cl, ctx);
					buffers.BindTextures(cl, ctx);

					const U8 state = Data::MaterialPBR::GetPipelineStateNumber(renderData.Assets.GetResources().get<Data::PBRFlags>(currentMaterial));
					if (currentState != state)
					{
						currentState = state;
						data.StatesTransparent[state].Bind(cl);
					}
				}
				ctx.Reset();

				renderData.Assets.GetResources().get<Resource::Mesh>(transparentGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
				ZE_DRAW_TAG_END(dev, cl);
				ZE_PERF_STOP();
			}
			ZE_PERF_STOP();

			ZE_DRAW_TAG_END(dev, cl);
			ZE_PERF_STOP();
			ZE_PERF_STOP();
		}
		cl.Close(dev);
		dev.ExecuteMain(cl);

		ZE_PERF_START("Lambertian - visibility clear");
		// Remove current visibility
		renderData.Registry.clear<InsideFrustumSolid, InsideFrustumNotSolid>();
		ZE_PERF_STOP();

		ZE_PERF_STOP();
	}
}