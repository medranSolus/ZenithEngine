#include "GFX/Pipeline/RenderPass/OutlineDraw.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::OutlineDraw
{
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Vertex, Binding::RangeFlag::Constant });
		desc.AddRange({ 1, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.AddRange({ sizeof(Float3), 2, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"SolidVS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::StencilWrite;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "OutlineDrawStencil");
		passData->StateStencil.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		psoDesc.SetShader(psoDesc.PS, L"SolidPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		psoDesc.FormatDS = PixelFormat::Unknown;
		ZE_PSO_SET_NAME(psoDesc, "OutlineDrawRender");
		passData->StateRender.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		passData->TransformBuffers.Exec([&dev](auto& x) { x.emplace_back(dev, nullptr, static_cast<U32>(sizeof(TransformBuffer)), true); });

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		// Clearing data on first usage
		cl.Open(dev);
		ZE_DRAW_TAG_BEGIN(cl, L"Outline Draw Clear", PixelVal::White);

		renderData.Buffers.ClearDSV(cl, ids.DepthStencil, 1.0f, 0);
		renderData.Buffers.ClearRTV(cl, ids.RenderTarget, { 0.0f, 0.0f, 0.0f, 0.0f });

		ZE_DRAW_TAG_END(cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);

		auto group = Data::GetRenderGroup<Data::RenderOutline>(renderData.Registry);
		U64 count = group.size();
		if (count)
		{
			const RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
			const CameraPBR& dynamicData = *reinterpret_cast<CameraPBR*>(renderData.DynamicData);
			const Matrix viewProjection = dynamicData.ViewProjection;
			const Vector cameraPos = Math::XMLoadFloat3(&dynamicData.CameraPos);

			// Compute visibility of objects inside camera view and sort them front-back
			Math::BoundingFrustum frustum(Math::XMLoadFloat4x4(&renderer.GetProjection()), false);
			frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&renderer.GetCameraRotation()), cameraPos);
			Utils::FrustumCulling<InsideFrustum, InsideFrustum>(renderData.Registry, renderData.Resources, group, frustum);

			auto visibleGroup = Data::GetVisibleRenderGroup<Data::RenderOutline, InsideFrustum>(renderData.Registry);
			count = visibleGroup.size();
			Utils::ViewSortAscending(visibleGroup, cameraPos);

			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<Matrix, TransformBuffer>(dev, data.TransformBuffers.Get(), count);
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			for (U64 i = 0, j = 0; i < count; ++j)
			{
				cl.Open(dev, data.StateStencil);
				ZE_DRAW_TAG_BEGIN(cl, (L"Outline Draw Batch_" + std::to_wstring(j)).c_str(), Pixel(0xF9, 0xE0, 0x76));
				ctx.BindingSchema.SetGraphics(cl);
				data.StateStencil.SetStencilRef(cl, 0xFF);
				renderData.Buffers.SetDSV(cl, ids.DepthStencil);

				ctx.SetFromEnd(1);
				auto& cbuffer = data.TransformBuffers.Get().at(j);
				cbuffer.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				TransformBuffer* buffer = reinterpret_cast<TransformBuffer*>(cbuffer.GetRegion());
				for (U32 k = 0, I = i; k < TransformBuffer::TRANSFORM_COUNT && I < count; ++k, ++I)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"MeshStencil_" + std::to_wstring(k)).c_str(), Pixel(0xC9, 0xBB, 0x8E));

					auto entity = visibleGroup[I];
					const auto& transform = visibleGroup.get<Data::TransformGlobal>(entity);
					buffer->Transforms[k] = viewProjection *
						Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

					Resource::Constant<U32> meshBatchId(dev, k);
					meshBatchId.Bind(cl, ctx);
					ctx.Reset();

					const auto& geometry = renderData.Resources.get<Data::Geometry>(visibleGroup.get<Data::MeshID>(entity).ID);
					geometry.Vertices.Bind(cl);
					geometry.Indices.Bind(cl);

					cl.DrawIndexed(dev, geometry.Indices.GetCount());
					ZE_DRAW_TAG_END(cl);
				}

				// Separate calls due to different RT/DS sizes
				data.StateRender.Bind(cl);
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetRTV(cl, ids.RenderTarget);

				ctx.SetFromEnd(1);
				cbuffer.Bind(cl, ctx);
				Resource::Constant<Float3> solidColor(dev, { 1.0f, 1.0f, 0.0f }); // Can be taken from mesh later
				solidColor.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				for (U32 k = 0; k < TransformBuffer::TRANSFORM_COUNT && i < count; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"MeshRender_" + std::to_wstring(k)).c_str(), Pixel(0xB9, 0xAB, 0x6E));

					Resource::Constant<U32> meshBatchId(dev, k);
					meshBatchId.Bind(cl, ctx);
					ctx.Reset();

					const auto& geometry = renderData.Resources.get<Data::Geometry>(visibleGroup.get<Data::MeshID>(visibleGroup[i]).ID);
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
			renderData.Registry.clear<InsideFrustum>();
		}
	}
}