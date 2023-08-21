#include "GFX/Pipeline/RenderPass/OutlineDraw.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::OutlineDraw
{
	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->StateStencil.Free(dev);
		execData->StateRender.Free(dev);
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ sizeof(Float3), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Solid color
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "SolidVS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::StencilWrite;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "OutlineDrawStencil");
		passData->StateStencil.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		psoDesc.SetShader(dev, psoDesc.PS, "SolidPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		psoDesc.FormatDS = PixelFormat::Unknown;
		ZE_PSO_SET_NAME(psoDesc, "OutlineDrawRender");
		passData->StateRender.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Outline Draw");
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		// Clearing data on first usage
		cl.Open(dev);
		ZE_DRAW_TAG_BEGIN(dev, cl, "Outline Draw Clear", PixelVal::White);
		renderData.Buffers.ClearDSV(cl, ids.DepthStencil, 1.0f, 0);
		renderData.Buffers.ClearRTV(cl, ids.RenderTarget, { 0.0f, 0.0f, 0.0f, 0.0f });
		ZE_DRAW_TAG_END(dev, cl);

		auto group = Data::GetRenderGroup<Data::RenderOutline>(renderData.Registry);
		U64 count = group.size();
		if (count)
		{
			ZE_PERF_GUARD("Outline Draw - outline present");

			const RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
			const CameraPBR& dynamicData = *reinterpret_cast<CameraPBR*>(renderData.DynamicData);
			const Matrix viewProjection = dynamicData.ViewProjection;
			const Vector cameraPos = Math::XMLoadFloat3(&dynamicData.CameraPos);

			// Compute visibility of objects inside camera view and sort them front-back
			ZE_PERF_START("Outline Draw - frustum culling");
			Math::BoundingFrustum frustum(Math::XMLoadFloat4x4(&renderer.GetProjection()), false);
			frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&renderer.GetCameraRotation()), cameraPos);
			Utils::FrustumCulling<InsideFrustum, InsideFrustum>(renderData.Registry, renderData.Assets.GetResources(), group, frustum);
			ZE_PERF_STOP();

			ZE_PERF_START("Outline Draw - view sort");
			auto visibleGroup = Data::GetVisibleRenderGroup<Data::RenderOutline, InsideFrustum>(renderData.Registry);
			count = visibleGroup.size();
			Utils::ViewSortAscending(visibleGroup, cameraPos);
			ZE_PERF_STOP();

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			auto& cbuffer = renderData.DynamicBuffers.Get();

			data.StateStencil.Bind(cl);
			ZE_DRAW_TAG_BEGIN(dev, cl, "Outline Draw Stencil", Pixel(0xF9, 0xE0, 0x76));
			ctx.BindingSchema.SetGraphics(cl);
			data.StateStencil.SetStencilRef(cl, 0xFF);
			renderData.Buffers.SetDSV(cl, ids.DepthStencil);

			ZE_PERF_START("Outline Draw Stencil - main loop");
			for (U64 i = 0; i < count; ++i)
			{
				ZE_PERF_GUARD("Outline Draw Stencil - single loop item");
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0xC9, 0xBB, 0x8E));

				EID entity = visibleGroup[i];
				const auto& transform = visibleGroup.get<Data::TransformGlobal>(entity);

				TransformBuffer transformBuffer;
				transformBuffer.Transform = viewProjection *
					Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

				auto& transformInfo = visibleGroup.get<InsideFrustum>(entity);
				transformInfo.Transform = cbuffer.Alloc(dev, &transformBuffer, sizeof(TransformBuffer));
				cbuffer.Bind(cl, ctx, transformInfo.Transform);
				ctx.Reset();

				renderData.Assets.GetResources().get<Resource::Mesh>(visibleGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
				ZE_DRAW_TAG_END(dev, cl);
			}
			ZE_PERF_STOP();
			ZE_DRAW_TAG_END(dev, cl);

			// Separate calls due to different RT/DS sizes
			data.StateRender.Bind(cl);
			ZE_DRAW_TAG_BEGIN(dev, cl, "Outline Draw", Pixel(0xCD, 0xD4, 0x6A));
			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetRTV(cl, ids.RenderTarget);

			ctx.SetFromEnd(0);
			Resource::Constant<Float3> solidColor(dev, { 1.0f, 1.0f, 0.0f }); // Can be taken from mesh later
			solidColor.Bind(cl, ctx);
			ctx.Reset();

			ZE_PERF_START("Outline Draw - main loop");
			for (U64 i = 0; i < count; ++i)
			{
				ZE_PERF_GUARD("Outline Draw - single loop item");
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0xB9, 0xAB, 0x6E));

				EID entity = visibleGroup[i];
				cbuffer.Bind(cl, ctx, visibleGroup.get<InsideFrustum>(entity).Transform);
				ctx.Reset();

				renderData.Assets.GetResources().get<Resource::Mesh>(visibleGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
				ZE_DRAW_TAG_END(dev, cl);
			}
			ZE_PERF_STOP();
			ZE_DRAW_TAG_END(dev, cl);

			// Remove current visibility
			ZE_PERF_START("Outline Draw - visibility clear");
			renderData.Registry.clear<InsideFrustum>();
			ZE_PERF_STOP();
		}
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}