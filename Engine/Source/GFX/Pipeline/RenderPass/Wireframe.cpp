#include "GFX/Pipeline/RenderPass/Wireframe.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::Wireframe
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
		psoDesc.SetShader(psoDesc.PS, L"SolidPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthReverse;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		psoDesc.FormatDS = formatDS;
		psoDesc.Topology = Resource::TopologyType::Line;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "Wireframe");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		passData->TransformBuffers.Exec([&dev](auto& x) { x.emplace_back(dev, nullptr, static_cast<U32>(sizeof(TransformBuffer)), true); });

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		auto group = Data::GetRenderGroup<Data::RenderWireframe>(renderData.Registry);
		U64 count = group.size();
		if (count)
		{
			const RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
			const CameraPBR& dynamicData = *reinterpret_cast<CameraPBR*>(renderData.DynamicData);
			const Matrix viewProjection = dynamicData.ViewProjection;

			// Compute visibility of objects inside camera view
			Math::BoundingFrustum frustum(Math::XMLoadFloat4x4(&renderer.GetProjection()), false);
			frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&renderer.GetCameraRotation()), Math::XMLoadFloat3(&dynamicData.CameraPos));
			Utils::FrustumCulling<InsideFrustum, InsideFrustum>(renderData.Registry, renderData.Resources, group, frustum);
			auto visibleGroup = Data::GetVisibleRenderGroup<Data::RenderWireframe, InsideFrustum>(renderData.Registry);
			count = visibleGroup.size();

			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<Matrix, TransformBuffer>(dev, data.TransformBuffers.Get(), count);

			Resources ids = *passData.Buffers.CastConst<Resources>();
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			for (U64 i = 0, j = 0; i < count; ++j)
			{
				cl.Open(dev, data.State);
				ZE_DRAW_TAG_BEGIN(cl, (L"Wireframe Batch_" + std::to_wstring(j)).c_str(), Pixel(0xBC, 0x54, 0x4B));
				ctx.BindingSchema.SetGraphics(cl);
				renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.DepthStencil);

				ctx.SetFromEnd(1);
				auto& cbuffer = data.TransformBuffers.Get().at(j);
				cbuffer.Bind(cl, ctx);
				Resource::Constant<Float3> solidColor(dev, { 1.0f, 1.0f, 1.0f }); // Can be taken from mesh later
				solidColor.Bind(cl, ctx);
				ctx.Reset();

				// Compute single batch
				TransformBuffer* buffer = reinterpret_cast<TransformBuffer*>(cbuffer.GetRegion());
				for (U32 k = 0; k < TransformBuffer::TRANSFORM_COUNT && i < count; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(cl, (L"Mesh_" + std::to_wstring(k)).c_str(), Pixel(0xE3, 0x24, 0x2B));

					auto entity = visibleGroup[i];
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

				ZE_DRAW_TAG_END(cl);
				cl.Close(dev);
				dev.ExecuteMain(cl);
			}
			// Remove current visibility
			renderData.Registry.clear<InsideFrustum>();
		}
	}
}