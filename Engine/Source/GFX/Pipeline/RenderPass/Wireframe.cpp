#include "GFX/Pipeline/RenderPass/Wireframe.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"
#include "GFX/Resource/Mesh.h"
#include "GFX/TransformBuffer.h"
#include "Data/Camera.h"

namespace ZE::GFX::Pipeline::RenderPass::Wireframe
{
	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, PassInitData* initData) noexcept
	{
		ZE_ASSERT(formats.size() == 2, "Incorrect size for Wireframe initialization formats!");
		return Initialize(dev, buildData, formats.at(0), formats.at(1));
	}

	PassDesc GetDesc(PixelFormat formatRT, PixelFormat formatDS) noexcept
	{
		PassDesc desc{ Base(CorePassType::Wireframe) };
		desc.InitializeFormats.reserve(2);
		desc.InitializeFormats.emplace_back(formatRT);
		desc.InitializeFormats.emplace_back(formatDS);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		return desc;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS) noexcept
	{
		auto passData = std::make_shared<ExecuteData>();

		Binding::SchemaDesc desc = {};
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform
		desc.AddRange({ sizeof(Float3), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Solid color
		ZE_EXPECT_RET_FAILED(passData->BindingIndex, buildData.BindingLib.AddDataBinding(dev, desc));

		Resource::PipelineStateDesc psoDesc = {};
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.VS, "SolidVS", buildData.ShaderCache));
		ZE_CODE_RET_FAILED_EXPECT(psoDesc.SetShader(dev, psoDesc.PS, "SolidPS", buildData.ShaderCache));
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthReverse;
		psoDesc.Culling = Resource::CullMode::Back;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		psoDesc.FormatDS = formatDS;
		psoDesc.Topology = Resource::TopologyType::Line;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "Wireframe");
		ZE_EXPECT_RET_FAILED(passData->State, Resource::PipelineStateGfx::Create(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex)));

		Data::AssureEntityPools<InsideFrustum>(Settings::DataBank.GetWorldData());
		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		auto view = Settings::DataBank.GetWorldData().view<Data::RenderWireframe, Data::TransformGlobal, Data::MaterialID, Data::MeshID>();
		if (view.begin() != view.end())
		{
			ZE_PERF_GUARD("Wireframe - present");
			const Matrix viewProjection = Math::XMLoadFloat4x4(&renderData.DynamicData.ViewProjectionTps);

			// Compute visibility of objects inside camera view
			ZE_PERF_START("Wireframe - frustum culling");
			Math::BoundingFrustum frustum = Data::GetFrustum(Math::XMLoadFloat4x4(&renderData.GraphData.Projection), Settings::MaxRenderDistance);
			frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&Settings::DataBank.GetWorldData().get<Data::TransformGlobal>(renderData.GraphData.CurrentCamera).Rotation),
				Math::XMLoadFloat3(&renderData.DynamicData.CameraPos));
			Utils::FrustumCulling<InsideFrustum, InsideFrustum>(view, frustum);
			ZE_PERF_STOP();

			auto visibleView = Settings::DataBank.GetWorldData().view<InsideFrustum, Data::RenderWireframe, Data::TransformGlobal, Data::MaterialID, Data::MeshID>();

			Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
			ExecuteData& data = *static_cast<ExecuteData*>(passData.ExecData.get());

			ZE_DRAW_TAG_BEGIN(dev, cl, "Wireframe", Pixel(0xBC, 0x54, 0x4B));
			renderData.Buffers.BeginRaster(cl, ids.RenderTarget, ids.DepthStencil);

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			ctx.BindingSchema.SetGraphics(cl);
			data.State.Bind(cl);

			ctx.SetFromEnd(0);
			Resource::Constant<Float3> solidColor; // Can be taken from mesh later
			ZE_EXPECT_RET_FAILED(solidColor, Resource::Constant<Float3>::Create(dev, { 1.0f, 1.0f, 1.0f }));
			solidColor.Bind(cl, ctx);
			ctx.Reset();

			auto& cbuffer = *renderData.DynamicBuffer;
			ZE_PERF_START("Wireframe - main loop");
			for (EID entity : visibleView)
			{
				ZE_PERF_GUARD("Wireframe - single loop item");
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(static_cast<U64>(entity))).c_str(), Pixel(0xE3, 0x24, 0x2B));

				const auto& transform = visibleView.get<Data::TransformGlobal>(entity);

				TransformBuffer transformBuffer = {};
				Math::XMStoreFloat4x4(&transformBuffer.TransformTps, viewProjection *
					Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale)));

				ZE_CODE_RET_FAILED_EXPECT(cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(TransformBuffer)));
				ctx.Reset();

				Settings::DataBank.GetAssetsData().get<Resource::Mesh>(visibleView.get<Data::MeshID>(entity).ID).Draw(dev, cl);
				ZE_DRAW_TAG_END(dev, cl);
			}
			renderData.Buffers.EndRaster(cl);
			ZE_PERF_STOP();
			ZE_DRAW_TAG_END(dev, cl);

			// Remove current visibility
			ZE_PERF_START("Wireframe - visibility clear");
			Settings::DataBank.GetWorldData().clear<InsideFrustum>();
			ZE_PERF_STOP();
			return true;
		}
		return false;
	}
}