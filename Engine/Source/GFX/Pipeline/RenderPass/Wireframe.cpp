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
		desc.AddRange({ 1, 0, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform
		desc.AddRange({ sizeof(Float3), 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Solid color
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

			Resources ids = *passData.Buffers.CastConst<Resources>();
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			cl.Open(dev, data.State);
			ZE_DRAW_TAG_BEGIN(dev, cl, L"Wireframe", Pixel(0xBC, 0x54, 0x4B));
			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.DepthStencil);

			ctx.SetFromEnd(1);
			Resource::Constant<Float3> solidColor(dev, { 1.0f, 1.0f, 1.0f }); // Can be taken from mesh later
			solidColor.Bind(cl, ctx);
			ctx.Reset();

			auto& cbuffer = renderData.DynamicBuffers.Get();
			for (U64 i = 0; i < count; ++i)
			{
				ZE_DRAW_TAG_BEGIN(dev, cl, (L"Mesh_" + std::to_wstring(i)).c_str(), Pixel(0xE3, 0x24, 0x2B));

				auto entity = visibleGroup[i];
				const auto& transform = visibleGroup.get<Data::TransformGlobal>(entity);

				TransformBuffer transformBuffer;
				transformBuffer.Transform = viewProjection *
					Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

				cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(TransformBuffer));
				ctx.Reset();

				const auto& geometry = renderData.Resources.get<Data::Geometry>(visibleGroup.get<Data::MeshID>(entity).ID);
				geometry.Vertices.Bind(cl);
				geometry.Indices.Bind(cl);

				cl.DrawIndexed(dev, geometry.Indices.GetCount());
				ZE_DRAW_TAG_END(dev, cl);
			}
			ZE_DRAW_TAG_END(dev, cl);
			cl.Close(dev);
			dev.ExecuteMain(cl);
			// Remove current visibility
			renderData.Registry.clear<InsideFrustum>();
		}
	}
}