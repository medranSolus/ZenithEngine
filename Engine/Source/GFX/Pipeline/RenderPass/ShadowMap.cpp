#include "GFX/Pipeline/RenderPass/ShadowMap.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Resource/Mesh.h"
#include "GFX/TransformBuffer.h"
#include "GFX/Vertex.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMap
{
#pragma pack(push, 1)
	struct ShaderConstantData
	{
		Float3 LightPos = {};
		float ParallaxScale = 0.0f;
		U32 Flags = 0;
	};
#pragma pack(pop)

	Status Initialize(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData,
		PixelFormat formatDS, PixelFormat formatRT, Matrix projection) noexcept
	{
		Binding::SchemaDesc desc = {};
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ sizeof(ShaderConstantData), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light shadow data
		desc.AddRange({ 4, 0, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Texture, normal, roughness+metal (not used), parallax
		desc.AddRange(buildData.DynamicDataRange, Resource::ShaderType::Vertex);
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		ZE_EXPECT_RET_FAILED_CODE(passData.BindingIndex, buildData.BindingLib.AddDataBinding(dev, desc));

		const auto& schema = buildData.BindingLib.GetSchema(passData.BindingIndex);
		Resource::PipelineStateDesc psoDesc = {};
		ZE_CODE_RET_FAILED(psoDesc.SetShader(dev, psoDesc.VS, "LambertDepthVS", buildData.ShaderCache));
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout = Vertex::GetLayout();
		ZE_PSO_SET_NAME(psoDesc, "ShadowMapDepth");
		ZE_EXPECT_RET_FAILED_CODE(passData.StateDepth, Resource::PipelineStateGfx::Create(dev, psoDesc, schema));

		ZE_CODE_RET_FAILED(psoDesc.SetShader(dev, psoDesc.VS, "LambertVS", buildData.ShaderCache));
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		const std::string shaderName = "ShadowPS";
		// Ignore flag UseSpecular as it does not have impact on shadows
		U8 stateIndex = Data::MaterialPBR::GetPipelineStateNumber(SHADOW_PERMUTATIONS) + 1;
		passData.StatesSolid = std::make_unique<Resource::PipelineStateGfx[]>(stateIndex);
		passData.StatesTransparent = std::make_unique<Resource::PipelineStateGfx[]>(stateIndex);

		while (stateIndex--)
		{
			const char* suffix = Data::MaterialPBR::DecodeShaderSuffix(Data::MaterialPBR::GetShaderFlagsForState(stateIndex));
			ZE_CODE_RET_FAILED(psoDesc.SetShader(dev, psoDesc.PS, (shaderName + suffix).c_str(), buildData.ShaderCache));

			psoDesc.DepthStencil = Resource::DepthStencilMode::DepthBefore;
			ZE_PSO_SET_NAME(psoDesc, "ShadowMapSolid" + std::string(suffix));
			ZE_EXPECT_RET_FAILED_CODE(passData.StatesSolid[stateIndex], Resource::PipelineStateGfx::Create(dev, psoDesc, schema));

			psoDesc.DepthStencil = Resource::DepthStencilMode::StencilOff;
			ZE_PSO_SET_NAME(psoDesc, "ShadowMapTransparent" + std::string(suffix));
			ZE_EXPECT_RET_FAILED_CODE(passData.StatesTransparent[stateIndex], Resource::PipelineStateGfx::Create(dev, psoDesc, schema));
		}

		Math::XMStoreFloat4x4(&passData.Projection, projection);
		Data::AssureEntityPools<InsideFrustumSolid, InsideFrustumNotSolid>(Settings::DataBank.GetWorldData());
		return {};
	}

	Expected<Matrix> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData,
		ExecuteData& data, const Resources& ids, const Float3& lightPos,
		const Float3& lightDir, const Math::BoundingFrustum& frustum) noexcept
	{
		// Clearing data on first usage
		ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Clear", PixelVal::Gray);
		renderData.Buffers.ClearDSV(cl, ids.Depth, 0.0f, 0);
		renderData.Buffers.ClearRTV(cl, ids.RenderTarget, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
		ZE_DRAW_TAG_END(dev, cl);

		// Prepare view-projection for shadow
		const Vector position = Math::XMLoadFloat3(&lightPos);
		const Vector direction = Math::XMLoadFloat3(&lightDir);
		const Matrix viewProjection = Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position, direction, Math::XMVector3Orthogonal(direction)) * Math::XMLoadFloat4x4(&data.Projection));

		auto view = Settings::DataBank.GetWorldData().view<Data::ShadowCaster, Data::TransformGlobal, Data::MaterialID, Data::MeshID>();
		if (view.begin() != view.end())
		{
			ZE_PERF_GUARD("Shadow Map - present");

			// Compute visibility of objects inside camera view
			ZE_PERF_START("Shadow Map - frustum culling");
			Utils::FrustumCulling<InsideFrustumSolid, InsideFrustumNotSolid>(view, frustum);
			ZE_PERF_STOP();

			auto solidView = Settings::DataBank.GetWorldData().view<InsideFrustumSolid, Data::ShadowCaster, Data::TransformGlobal, Data::MaterialID, Data::MeshID>();
			auto transparentView = Settings::DataBank.GetWorldData().view<InsideFrustumNotSolid, Data::ShadowCaster, Data::TransformGlobal, Data::MaterialID, Data::MeshID>();

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			auto& cbuffer = *renderData.DynamicBuffer;

			EID currentMaterial = INVALID_EID;
			U8 currentState = UINT8_MAX;
			Resource::Constant<ShaderConstantData> shadowData;
			ZE_EXPECT_RET_FAILED(shadowData, Resource::Constant<ShaderConstantData>::Create(dev, { lightPos, 0.0f, 0 }));
			if (solidView.begin() != solidView.end())
			{
				ZE_PERF_GUARD("Shadow Map - solid present");

				ZE_PERF_START("Shadow Map - solid view sort");
				Utils::ViewSortAscending(solidView, position);
				ZE_PERF_STOP();

				// Depth pre-pass
				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Depth", Pixel(0x98, 0x9F, 0xA7));
				renderData.Buffers.BeginRasterDepthOnly(cl, ids.Depth);
				ctx.BindingSchema.SetGraphics(cl);
				data.StateDepth.Bind(cl);

				ctx.SetFromEnd(1);
				renderData.BindRendererDynamicData(cl, ctx);
				ctx.Reset();

				ZE_PERF_START("Shadow Map Depth - main loop");
				for (EID entity : solidView)
				{
					ZE_PERF_GUARD("Shadow Map Depth - single loop item");
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(static_cast<U64>(entity))).c_str(), PixelVal::Gray);

					const auto& transform = solidView.get<Data::TransformGlobal>(entity);

					ModelTransformBuffer transformBuffer = {};
					const Matrix modelTransform = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
					Math::XMStoreFloat4x4(&transformBuffer.ModelTps, modelTransform);
					Math::XMStoreFloat4x4(&transformBuffer.ModelViewProjectionTps, viewProjection * modelTransform);

					auto& transformInfo = solidView.get<InsideFrustumSolid>(entity);
					ZE_EXPECT_RET_FAILED(transformInfo.Transform, cbuffer.Alloc(dev, &transformBuffer, sizeof(ModelTransformBuffer)));
					cbuffer.Bind(cl, ctx, transformInfo.Transform);
					ctx.Reset();

					Settings::DataBank.GetAssetsData().get<Resource::Mesh>(solidView.get<Data::MeshID>(entity).ID).Draw(dev, cl);
					ZE_DRAW_TAG_END(dev, cl);
				}
				renderData.Buffers.EndRaster(cl);
				ZE_PERF_STOP();
				ZE_DRAW_TAG_END(dev, cl);

				// Sort by pipeline state
				ZE_PERF_START("Shadow Map - solid material sort");
				Settings::DataBank.GetWorldData().sort<Data::MaterialID>([&](const auto& m1, const auto& m2) -> bool
					{
						const U8 state1 = Data::MaterialPBR::GetPipelineStateNumber({ static_cast<U8>(Settings::DataBank.GetAssetsData().get<Data::PBRFlags>(m1.ID) & SHADOW_PERMUTATIONS) });
						const U8 state2 = Data::MaterialPBR::GetPipelineStateNumber({ static_cast<U8>(Settings::DataBank.GetAssetsData().get<Data::PBRFlags>(m2.ID) & SHADOW_PERMUTATIONS) });
						return state1 < state2;
					});
				solidView.use<Data::MaterialID>();
				currentState = Data::MaterialPBR::GetPipelineStateNumber({ static_cast<U8>(Settings::DataBank.GetAssetsData().get<Data::PBRFlags>(solidView.get<Data::MaterialID>(solidView.front()).ID) & SHADOW_PERMUTATIONS)});
				ZE_PERF_STOP();

				// Solid pass
				ZE_PERF_START("Shadow Map Solid");
				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Solid", Pixel(0x79, 0x82, 0x8D));
				renderData.Buffers.BeginRaster(cl, ids.RenderTarget, ids.Depth);
				ctx.BindingSchema.SetGraphics(cl);
				data.StatesSolid[currentState].Bind(cl);

				ctx.SetFromEnd(1);
				renderData.BindRendererDynamicData(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				ZE_PERF_START("Shadow Map Solid - main loop");
				for (EID entity : solidView)
				{
					ZE_PERF_GUARD("Shadow Map Solid - single loop item");
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(static_cast<U64>(entity))).c_str(), Pixel(0x5D, 0x5E, 0x61));

					cbuffer.Bind(cl, ctx, solidView.get<InsideFrustumSolid>(entity).Transform);

					const Data::MaterialID material = solidView.get<Data::MaterialID>(entity);
					if (currentMaterial != material.ID)
					{
						currentMaterial = material.ID;

						const auto& matData = Settings::DataBank.GetAssetsData().get<Data::MaterialPBR>(currentMaterial);
						ZE_CODE_RET_FAILED_EXPECT(shadowData.Set(dev, { lightPos, matData.ParallaxScale, matData.Flags }));
						shadowData.Bind(cl, ctx);
						Settings::DataBank.GetAssetsData().get<Data::MaterialBuffersPBR>(currentMaterial).BindTextures(cl, ctx);

						const U8 state = Data::MaterialPBR::GetPipelineStateNumber({ static_cast<U8>(Settings::DataBank.GetAssetsData().get<Data::PBRFlags>(currentMaterial) & SHADOW_PERMUTATIONS) });
						if (currentState != state)
						{
							currentState = state;
							data.StatesSolid[state].Bind(cl);
						}
					}
					ctx.Reset();

					Settings::DataBank.GetAssetsData().get<Resource::Mesh>(solidView.get<Data::MeshID>(entity).ID).Draw(dev, cl);
					ZE_DRAW_TAG_END(dev, cl);
				}
				ZE_PERF_STOP();

				renderData.Buffers.EndRaster(cl);
				ZE_DRAW_TAG_END(dev, cl);
				ZE_PERF_STOP();

				currentMaterial = INVALID_EID;
				currentState = UINT8_MAX;
			}

			// Transparent pass
			if (transparentView.begin() != transparentView.end())
			{
				ZE_PERF_GUARD("Shadow Map - transparent present");

				ZE_PERF_START("Shadow Map - transparent view sort");
				Utils::ViewSortDescending(transparentView, position);
				ZE_PERF_STOP();

				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Transparent", Pixel(0x79, 0x82, 0x8D));
				renderData.Buffers.BeginRaster(cl, ids.RenderTarget, ids.Depth);
				ctx.BindingSchema.SetGraphics(cl);

				ctx.SetFromEnd(1);
				renderData.BindRendererDynamicData(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				ZE_PERF_START("Shadow Map Transparent - main loop");
				for (EID entity : transparentView)
				{
					ZE_PERF_GUARD("Shadow Map Transparent - single loop item");
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(static_cast<U64>(entity))).c_str(), Pixel(0x5D, 0x5E, 0x61));

					const auto& transform = transparentView.get<Data::TransformGlobal>(entity);

					ModelTransformBuffer transformBuffer = {};
					const Matrix modelTransform = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
					Math::XMStoreFloat4x4(&transformBuffer.ModelTps, modelTransform);
					Math::XMStoreFloat4x4(&transformBuffer.ModelViewProjectionTps, viewProjection * modelTransform);
					ZE_CODE_RET_FAILED_EXPECT(cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(ModelTransformBuffer)));

					const Data::MaterialID material = transparentView.get<Data::MaterialID>(entity);
					if (currentMaterial != material.ID)
					{
						currentMaterial = material.ID;

						const auto& matData = Settings::DataBank.GetAssetsData().get<Data::MaterialPBR>(material.ID);
						ZE_CODE_RET_FAILED_EXPECT(shadowData.Set(dev, { lightPos, matData.ParallaxScale, matData.Flags }));
						shadowData.Bind(cl, ctx);
						Settings::DataBank.GetAssetsData().get<Data::MaterialBuffersPBR>(material.ID).BindTextures(cl, ctx);

						const U8 state = Data::MaterialPBR::GetPipelineStateNumber({ static_cast<U8>(Settings::DataBank.GetAssetsData().get<Data::PBRFlags>(currentMaterial) & SHADOW_PERMUTATIONS) });
						if (currentState != state)
						{
							currentState = state;
							data.StatesTransparent[state].Bind(cl);
						}
					}
					ctx.Reset();

					Settings::DataBank.GetAssetsData().get<Resource::Mesh>(transparentView.get<Data::MeshID>(entity).ID).Draw(dev, cl);
					ZE_DRAW_TAG_END(dev, cl);
				}
				renderData.Buffers.EndRaster(cl);
				ZE_PERF_STOP();
				ZE_DRAW_TAG_END(dev, cl);
			}
			// Remove current visibility indication
			ZE_PERF_START("Shadow Map - visibility clear");
			Settings::DataBank.GetWorldData().clear<InsideFrustumSolid, InsideFrustumNotSolid>();
			ZE_PERF_STOP();
		}
		return viewProjection;
	}
}