#include "GFX/Pipeline/RenderPass/ShadowMapCube.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Resource/Mesh.h"
#include "GFX/TransformBuffer.h"
#include "GFX/Vertex.h"
#include "Data/Camera.h"

namespace ZE::GFX::Pipeline::RenderPass::ShadowMapCube
{
#pragma pack(push, 1)
	struct ShaderConstantData
	{
		Float3 LightPos = {};
		float ParallaxScale = 0.0f;
		U32 Flags = 0;
	};
#pragma pack(pop)

	Status Initialize(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat formatDS, PixelFormat formatRT) noexcept
	{
		Binding::SchemaDesc desc = {};
		desc.AddRange({ 1, 0, 4, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ sizeof(ShaderConstantData), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light shadow data
		desc.AddRange({ 4, 0, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Texture, normal, roughness+metal (not used), parallax
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Geometry, Binding::RangeFlag::CBV }); // Cube view buffer
		desc.AddRange(buildData.DynamicDataRange, Resource::ShaderType::Geometry);
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		ZE_EXPECT_RET_FAILED_CODE(passData.BindingIndex, buildData.BindingLib.AddDataBinding(dev, desc));

		const auto& schema = buildData.BindingLib.GetSchema(passData.BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		ZE_CODE_RET_FAILED(psoDesc.SetShader(dev, psoDesc.VS, "ShadowCubeDepthVS", buildData.ShaderCache));
		ZE_CODE_RET_FAILED(psoDesc.SetShader(dev, psoDesc.GS, "ShadowCubeDepthGS", buildData.ShaderCache));
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout = Vertex::GetLayout();
		ZE_PSO_SET_NAME(psoDesc, "ShadowMapCubeDepth");
		ZE_EXPECT_RET_FAILED_CODE(passData.StateDepth, Resource::PipelineStateGfx::Create(dev, psoDesc, schema));

		ZE_CODE_RET_FAILED(psoDesc.SetShader(dev, psoDesc.VS, "ShadowCubeVS", buildData.ShaderCache));
		ZE_CODE_RET_FAILED(psoDesc.SetShader(dev, psoDesc.GS, "ShadowCubeGS", buildData.ShaderCache));
		psoDesc.RenderTargetsCount = 6;
		for (U8 i = 0; i < psoDesc.RenderTargetsCount; ++i)
			psoDesc.FormatsRT[i] = formatRT;
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
			ZE_PSO_SET_NAME(psoDesc, "ShadowMapCubeSolid" + std::string(suffix));
			ZE_EXPECT_RET_FAILED_CODE(passData.StatesSolid[stateIndex], Resource::PipelineStateGfx::Create(dev, psoDesc, schema));

			psoDesc.DepthStencil = Resource::DepthStencilMode::StencilOff;
			ZE_PSO_SET_NAME(psoDesc, "ShadowMapCubeTransparent" + std::string(suffix));
			ZE_EXPECT_RET_FAILED_CODE(passData.StatesTransparent[stateIndex], Resource::PipelineStateGfx::Create(dev, psoDesc, schema));
		}

		Math::XMStoreFloat4x4(&passData.Projection, Data::GetProjectionMatrix({ static_cast<float>(M_PI_2), 1.0f, 0.0001f }));
		Data::AssureEntityPools<Solid, Transparent>(Settings::DataBank.GetWorldData());
		return {};
	}

	Status Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData,
		ExecuteData& data, const Resources& ids, const Float3& lightPos, float lightVolume) noexcept
	{
		// Clearing data on first usage
		ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Cube Clear", PixelVal::Gray);
		renderData.Buffers.ClearDSV(cl, ids.Depth, 0.0f, 0);
		renderData.Buffers.ClearRTV(cl, ids.RenderTarget, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
		ZE_DRAW_TAG_END(dev, cl);

		auto view = Settings::DataBank.GetWorldData().view<Data::ShadowCaster, Data::TransformGlobal, Data::MaterialID, Data::MeshID>();
		if (view.begin() != view.end())
		{
			ZE_PERF_GUARD("Shadow Map Cube - present");
			// Prepare view-projections for casting onto 6 faces
			CubeViewBuffer viewBuffer = {};
			const Vector position = Math::XMLoadFloat3(&lightPos);
			const Vector up = { 0.0f, 1.0f, 0.0f, 0.0f };
			const Matrix projection = Math::XMLoadFloat4x4(&data.Projection);
			// +x
			Math::XMStoreFloat4x4(viewBuffer.ViewProjectionTps, Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 1.0f, 0.0f, 0.0f, 0.0f }, up) * projection));
			// -x
			Math::XMStoreFloat4x4(viewBuffer.ViewProjectionTps + 1, Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ -1.0f, 0.0f, 0.0f, 0.0f }, up) * projection));
			// +y
			Math::XMStoreFloat4x4(viewBuffer.ViewProjectionTps + 2, Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }) * projection));
			// -y
			Math::XMStoreFloat4x4(viewBuffer.ViewProjectionTps + 3, Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 0.0f, -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }) * projection));
			// +z
			Math::XMStoreFloat4x4(viewBuffer.ViewProjectionTps + 4, Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 0.0f, 0.0f, 1.0f, 0.0f }, up) * projection));
			// -z
			Math::XMStoreFloat4x4(viewBuffer.ViewProjectionTps + 5, Math::XMMatrixTranspose(Math::XMMatrixLookToLH(position,
				{ 0.0f, 0.0f, -1.0f, 0.0f }, up) * projection));

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			auto& cbuffer = *renderData.DynamicBuffer;
			Resource::DynamicBufferAlloc cubeBufferInfo = {};
			ZE_EXPECT_RET_FAILED_CODE(cubeBufferInfo, cbuffer.Alloc(dev, &viewBuffer, sizeof(CubeViewBuffer)));

			// Split into groups based on materials and if inside light volume
			ZE_PERF_START("Shadow Map Cube - visibility group split loop");
			const Math::BoundingSphere lightSphere(lightPos, lightVolume);
			for (EID entity : view)
			{
				ZE_PERF_GUARD("Shadow Map Cube - visibility group split single loop item");
				const auto& transform = view.get<Data::TransformGlobal>(entity);

				Math::BoundingBox box = Settings::DataBank.GetAssetsData().get<Math::BoundingBox>(view.get<Data::MeshID>(entity).ID);
				box.Transform(box, Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

				if (box.Intersects(lightSphere))
				{
					if (Settings::DataBank.GetAssetsData().all_of<Data::MaterialTransparent>(view.get<Data::MaterialID>(entity).ID))
						Settings::DataBank.GetWorldData().emplace<Transparent>(entity);
					else
						Settings::DataBank.GetWorldData().emplace<Solid>(entity);
				}
			}
			ZE_PERF_STOP();

			auto solidView = Settings::DataBank.GetWorldData().view<Solid, Data::ShadowCaster, Data::TransformGlobal, Data::MaterialID, Data::MeshID>();
			auto transparentView = Settings::DataBank.GetWorldData().view<Transparent, Data::ShadowCaster, Data::TransformGlobal, Data::MaterialID, Data::MeshID>();

			EID currentMaterial = INVALID_EID;
			U8 currentState = UINT8_MAX;
			Resource::Constant<ShaderConstantData> shadowData;
			ZE_EXPECT_RET_FAILED_CODE(shadowData, Resource::Constant<ShaderConstantData>::Create(dev, ShaderConstantData{ lightPos, 0.0f, 0 }));
			if (solidView.begin() != solidView.end())
			{
				ZE_PERF_GUARD("Shadow Map Cube - solid present");

				ZE_PERF_START("Shadow Map Cube - solid view sort");
				Utils::ViewSortAscending(solidView, position);
				ZE_PERF_STOP();

				// Depth pre-pass
				ZE_PERF_START("Shadow Map Cube Depth");
				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Cube Depth", Pixel(0x75, 0x7C, 0x88));
				renderData.Buffers.BeginRasterDepthOnly(cl, ids.Depth);
				ctx.BindingSchema.SetGraphics(cl);
				data.StateDepth.Bind(cl);

				ctx.SetFromEnd(2);
				cbuffer.Bind(cl, ctx, cubeBufferInfo);
				renderData.BindRendererDynamicData(cl, ctx);
				ctx.Reset();

				ZE_PERF_START("Shadow Map Cube Depth - main loop");
				for (EID entity : solidView)
				{
					ZE_PERF_GUARD("Shadow Map Cube Depth - single loop item");
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(static_cast<U64>(entity))).c_str(), PixelVal::Gray);

					const auto& transform = solidView.get<Data::TransformGlobal>(entity);

					TransformBuffer transformBuffer = {};
					Math::XMStoreFloat4x4(&transformBuffer.TransformTps, Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale)));

					auto& transformInfo = solidView.get<Solid>(entity);
					ZE_EXPECT_RET_FAILED_CODE(transformInfo.Transform, cbuffer.Alloc(dev, &transformBuffer, sizeof(TransformBuffer)));
					cbuffer.Bind(cl, ctx, transformInfo.Transform);
					ctx.Reset();

					Settings::DataBank.GetAssetsData().get<Resource::Mesh>(solidView.get<Data::MeshID>(entity).ID).Draw(dev, cl);
					ZE_DRAW_TAG_END(dev, cl);
				}
				ZE_PERF_STOP();

				renderData.Buffers.EndRaster(cl);
				ZE_DRAW_TAG_END(dev, cl);
				ZE_PERF_STOP();

				// Sort by pipeline state
				ZE_PERF_START("Shadow Map Cube - solid material sort");
				Settings::DataBank.GetWorldData().sort<Data::MaterialID>([&](const auto& m1, const auto& m2) -> bool
					{
						const U8 state1 = Data::MaterialPBR::GetPipelineStateNumber({ static_cast<U8>(Settings::DataBank.GetAssetsData().get<Data::PBRFlags>(m1.ID) & SHADOW_PERMUTATIONS) });
						const U8 state2 = Data::MaterialPBR::GetPipelineStateNumber({ static_cast<U8>(Settings::DataBank.GetAssetsData().get<Data::PBRFlags>(m2.ID) & SHADOW_PERMUTATIONS) });
						return state1 < state2;
					});
				solidView.use<Data::MaterialID>(); // Force view to iterate in MaterialID order
				currentState = Data::MaterialPBR::GetPipelineStateNumber({ static_cast<U8>(Settings::DataBank.GetAssetsData().get<Data::PBRFlags>(solidView.get<Data::MaterialID>(solidView.front()).ID) & SHADOW_PERMUTATIONS)});
				ZE_PERF_STOP();

				// Solid pass
				ZE_PERF_START("Shadow Map Cube Solid");
				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Cube Solid", Pixel(0x52, 0xB2, 0xBF));
				renderData.Buffers.BeginRaster(cl, ids.RenderTarget, ids.Depth);
				ctx.BindingSchema.SetGraphics(cl);
				data.StatesSolid[currentState].Bind(cl);

				ctx.SetFromEnd(2);
				cbuffer.Bind(cl, ctx, cubeBufferInfo);
				renderData.BindRendererDynamicData(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				ZE_PERF_START("Shadow Map Cube Solid - main loop");
				for (EID entity : solidView)
				{
					ZE_PERF_GUARD("Shadow Map Cube Solid - single loop item");
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(static_cast<U64>(entity))).c_str(), Pixel(0x01, 0x60, 0x64));

					cbuffer.Bind(cl, ctx, solidView.get<Solid>(entity).Transform);

					const Data::MaterialID material = solidView.get<Data::MaterialID>(entity);
					if (currentMaterial != material.ID)
					{
						currentMaterial = material.ID;

						const auto& matData = Settings::DataBank.GetAssetsData().get<Data::MaterialPBR>(currentMaterial);
						ZE_CODE_RET_FAILED(shadowData.Set(dev, ShaderConstantData{ lightPos, matData.ParallaxScale, matData.Flags }));
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
				ZE_PERF_GUARD("Shadow Map Cube - transparent present");

				ZE_PERF_START("Shadow Map Cube - transparent view sort");
				Utils::ViewSortDescending(transparentView, position);
				ZE_PERF_STOP();

				ZE_PERF_START("Shadow Map Cube Transparent");
				ZE_DRAW_TAG_BEGIN(dev, cl, "Shadow Map Cube Transparent", Pixel(0x52, 0xB2, 0xBF));
				renderData.Buffers.BeginRaster(cl, ids.RenderTarget, ids.Depth);
				ctx.BindingSchema.SetGraphics(cl);

				ctx.SetFromEnd(2);
				cbuffer.Bind(cl, ctx, cubeBufferInfo);
				renderData.BindRendererDynamicData(cl, ctx);
				renderData.SettingsBuffer.Bind(cl, ctx);
				ctx.Reset();

				ZE_PERF_START("Shadow Map Cube Transparent - main loop");
				for (EID entity : transparentView)
				{
					ZE_PERF_GUARD("Shadow Map Cube Transparent - single loop item");
					ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(static_cast<U64>(entity))).c_str(), Pixel(0x01, 0x60, 0x64));

					const auto& transform = transparentView.get<Data::TransformGlobal>(entity);

					TransformBuffer transformBuffer = {};
					Math::XMStoreFloat4x4(&transformBuffer.TransformTps, Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale)));
					ZE_CODE_RET_FAILED(cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(TransformBuffer)));

					const Data::MaterialID material = transparentView.get<Data::MaterialID>(entity);
					if (currentMaterial != material.ID)
					{
						currentMaterial = material.ID;

						const auto& matData = Settings::DataBank.GetAssetsData().get<Data::MaterialPBR>(material.ID);
						ZE_CODE_RET_FAILED(shadowData.Set(dev, ShaderConstantData{ lightPos, matData.ParallaxScale, matData.Flags }));
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
				ZE_PERF_STOP();

				renderData.Buffers.EndRaster(cl);
				ZE_DRAW_TAG_END(dev, cl);
				ZE_PERF_STOP();
			}
			// Remove current material indication
			ZE_PERF_START("Shadow Map Cube - visibility clear");
			Settings::DataBank.GetWorldData().clear<Solid, Transparent>();
			ZE_PERF_STOP();
		}
		return {};
	}
}