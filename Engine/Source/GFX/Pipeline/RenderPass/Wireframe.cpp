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
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform
		desc.AddRange({ sizeof(Float3), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Solid color
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "SolidVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, "SolidPS", buildData.ShaderCache);
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

		auto group = Data::GetRenderGroup<Data::RenderWireframe>();
		U64 count = group.size();
		if (count)
		{
			ZE_PERF_GUARD("Wireframe - present");
			const RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
			const CameraPBR& dynamicData = *reinterpret_cast<CameraPBR*>(renderData.DynamicData);
			const Matrix viewProjection = Math::XMLoadFloat4x4(&dynamicData.ViewProjectionTps);

			// Compute visibility of objects inside camera view
			ZE_PERF_START("Wireframe - frustum culling");
			Math::BoundingFrustum frustum = Data::GetFrustum(Math::XMLoadFloat4x4(&renderer.GetProjection()), Settings::MaxRenderDistance);
			frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&renderer.GetCameraRotation()), Math::XMLoadFloat3(&dynamicData.CameraPos));
			Utils::FrustumCulling<InsideFrustum, InsideFrustum>(group, frustum);
			ZE_PERF_STOP();

			auto visibleGroup = Data::GetVisibleRenderGroup<Data::RenderWireframe, InsideFrustum>();
			count = visibleGroup.size();

			Resources ids = *passData.Buffers.CastConst<Resources>();
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			cl.Open(dev, data.State);
			ZE_DRAW_TAG_BEGIN(dev, cl, "Wireframe", Pixel(0xBC, 0x54, 0x4B));
			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.DepthStencil);

			ctx.SetFromEnd(0);
			Resource::Constant<Float3> solidColor(dev, { 1.0f, 1.0f, 1.0f }); // Can be taken from mesh later
			solidColor.Bind(cl, ctx);
			ctx.Reset();

			auto& cbuffer = *renderData.DynamicBuffer;
			ZE_PERF_START("Wireframe - main loop");
			for (U64 i = 0; i < count; ++i)
			{
				ZE_PERF_GUARD("Wireframe - single loop item");
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0xE3, 0x24, 0x2B));

				auto entity = visibleGroup[i];
				const auto& transform = visibleGroup.get<Data::TransformGlobal>(entity);

				TransformBuffer transformBuffer;
				Math::XMStoreFloat4x4(&transformBuffer.TransformTps, viewProjection *
					Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale)));

				cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(TransformBuffer));
				ctx.Reset();

				Settings::Data.get<Resource::Mesh>(visibleGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
				ZE_DRAW_TAG_END(dev, cl);
			}
			ZE_PERF_STOP();
			ZE_DRAW_TAG_END(dev, cl);
			cl.Close(dev);
			dev.ExecuteMain(cl);

			// Remove current visibility
			ZE_PERF_START("Wireframe - visibility clear");
			Settings::Data.clear<InsideFrustum>();
			ZE_PERF_STOP();
		}
	}
}