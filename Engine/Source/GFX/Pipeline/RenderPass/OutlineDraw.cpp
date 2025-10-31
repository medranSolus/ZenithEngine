#include "GFX/Pipeline/RenderPass/OutlineDraw.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::OutlineDraw
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 2, "Incorrect size for OutlineDraw initialization formats!");
		return Initialize(dev, buildData, formats.at(0), formats.at(1));
	}

	PassDesc GetDesc(PixelFormat formatRT, PixelFormat formatDS) noexcept
	{
		PassDesc desc{ Base(CorePassType::OutlineDraw) };
		desc.InitializeFormats.reserve(2);
		desc.InitializeFormats.emplace_back(formatRT);
		desc.InitializeFormats.emplace_back(formatDS);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Clean = Clean;
		return desc;
	}

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		syncStatus.SyncMain(dev);
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->StateStencil.Free(dev);
		execData->StateRender.Free(dev);
		delete execData;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS)
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

		Settings::AssureEntityPools<InsideFrustum>();
		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Outline Draw");

		auto group = Data::GetRenderGroup<Data::RenderOutline>();
		U64 count = group.size();
		if (count)
		{
			ZE_PERF_GUARD("Outline Draw - outline present");
			Resources ids = *passData.Resources.CastConst<Resources>();
			ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

			const Matrix viewProjection = Math::XMLoadFloat4x4(&renderData.DynamicData.ViewProjectionTps);
			const Vector cameraPos = Math::XMLoadFloat3(&renderData.DynamicData.CameraPos);

			// Compute visibility of objects inside camera view and sort them front-back
			ZE_PERF_START("Outline Draw - frustum culling");
			Math::BoundingFrustum frustum = Data::GetFrustum(Math::XMLoadFloat4x4(&renderData.GraphData.Projection), Settings::MaxRenderDistance);
			frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&Settings::Data.get<Data::TransformGlobal>(renderData.GraphData.CurrentCamera).Rotation), cameraPos);
			Utils::FrustumCulling<InsideFrustum, InsideFrustum>(group, frustum);
			ZE_PERF_STOP();

			ZE_PERF_START("Outline Draw - view sort");
			auto visibleGroup = Data::GetVisibleRenderGroup<Data::RenderOutline, InsideFrustum>();
			count = visibleGroup.size();
			Utils::ViewSortAscending(visibleGroup, cameraPos);
			ZE_PERF_STOP();

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			auto& cbuffer = *renderData.DynamicBuffer;

			ZE_DRAW_TAG_BEGIN(dev, cl, "Outline Draw Stencil", Pixel(0xF9, 0xE0, 0x76));
			renderData.Buffers.BeginRasterDepthOnly(cl, ids.DepthStencil);

			ctx.BindingSchema.SetGraphics(cl);
			data.StateStencil.Bind(cl);
			data.StateStencil.SetStencilRef(cl, 0xFF);

			ZE_PERF_START("Outline Draw Stencil - main loop");
			for (U64 i = 0; i < count; ++i)
			{
				ZE_PERF_GUARD("Outline Draw Stencil - single loop item");
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0xC9, 0xBB, 0x8E));

				EID entity = visibleGroup[i];
				const auto& transform = visibleGroup.get<Data::TransformGlobal>(entity);

				TransformBuffer transformBuffer = {};
				Math::XMStoreFloat4x4(&transformBuffer.TransformTps, viewProjection *
					Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale)));

				auto& transformInfo = visibleGroup.get<InsideFrustum>(entity);
				transformInfo.Transform = cbuffer.Alloc(dev, &transformBuffer, sizeof(TransformBuffer));
				cbuffer.Bind(cl, ctx, transformInfo.Transform);
				ctx.Reset();

				Settings::Data.get<Resource::Mesh>(visibleGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
				ZE_DRAW_TAG_END(dev, cl);
			}
			renderData.Buffers.EndRaster(cl);
			ZE_PERF_STOP();
			ZE_DRAW_TAG_END(dev, cl);

			// Separate calls due to different RT/DS sizes
			ZE_DRAW_TAG_BEGIN(dev, cl, "Outline Draw", Pixel(0xCD, 0xD4, 0x6A));
			renderData.Buffers.BeginRaster(cl, ids.RenderTarget);
			ctx.BindingSchema.SetGraphics(cl);
			data.StateRender.Bind(cl);

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

				Settings::Data.get<Resource::Mesh>(visibleGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
				ZE_DRAW_TAG_END(dev, cl);
			}
			renderData.Buffers.EndRaster(cl);
			ZE_PERF_STOP();
			ZE_DRAW_TAG_END(dev, cl);

			// Remove current visibility
			ZE_PERF_START("Outline Draw - visibility clear");
			Settings::Data.clear<InsideFrustum>();
			ZE_PERF_STOP();
			return true;
		}
		return false;
	}
}