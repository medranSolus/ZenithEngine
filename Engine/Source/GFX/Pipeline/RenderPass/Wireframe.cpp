#include "GFX/Pipeline/RenderPass/Wireframe.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::Wireframe
{
	Data* Setup(Device& dev, RendererBuildData& buildData, Info::World& worldData, PixelFormat formatRT, PixelFormat formatDS)
	{
		Data* passData = new Data{ worldData };

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
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		ZE_PSO_SET_NAME(psoDesc, "Wireframe");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		passData->TransformBuffers.emplace_back(dev, nullptr, static_cast<U32>(sizeof(TransformBuffer)), true);

		return passData;
	}

	void Execute(RendererExecuteData& renderData, PassData& passData)
	{
		Data& data = *reinterpret_cast<Data*>(passData.OptData);

		if (data.World.WireframeInfo.Size)
		{
			// Resize temporary buffer for transform data
			Utils::ResizeTransformBuffers<Matrix, TransformBuffer>(renderData.Dev, data.TransformBuffers, data.World.WireframeInfo.Size);

			Resources ids = *passData.Buffers.CastConst<Resources>();
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			const auto& transforms = data.World.ActiveScene->TransformsGlobal;
			const auto& geometries = data.World.ActiveScene->Geometries;

			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			for (U64 i = 0, j = 0; i < data.World.WireframeInfo.Size; ++j)
			{
				renderData.CL.Open(renderData.Dev, data.State);
				ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Wireframe Batch_" + std::to_wstring(j)).c_str(), Pixel(0xBC, 0x54, 0x4B));
				ctx.BindingSchema.SetGraphics(renderData.CL);
				renderData.Buffers.SetOutput(renderData.CL, ids.RenderTarget, ids.DepthStencil);

				ctx.SetFromEnd(1);
				auto& cbuffer = data.TransformBuffers.at(j);
				cbuffer.Bind(renderData.CL, ctx);
				Resource::Constant<Float3> solidColor(renderData.Dev, { 1.0f, 1.0f, 1.0f }); // Can be taken from mesh later
				solidColor.Bind(renderData.CL, ctx);
				ctx.Reset();

				// Compute single batch
				TransformBuffer* buffer = reinterpret_cast<TransformBuffer*>(cbuffer.GetRegion());
				for (U32 k = 0; k < TransformBuffer::TRANSFORM_COUNT && i < data.World.WireframeInfo.Size; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(renderData.CL, (L"Mesh_" + std::to_wstring(k)).c_str(), Pixel(0xE3, 0x24, 0x2B));

					const auto& info = data.World.Wireframes[i];
					const auto& transform = transforms[info.TransformIndex];
					buffer->Transforms[k] = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale)) * data.World.DynamicData.ViewProjection;

					Resource::Constant<U32> meshBatchId(renderData.Dev, k);
					meshBatchId.Bind(renderData.CL, ctx);
					ctx.Reset();

					const auto& geometry = geometries[info.GeometryIndex];
					geometry.Vertices.Bind(renderData.CL);
					geometry.Indices.Bind(renderData.CL);

					renderData.CL.DrawIndexed(renderData.Dev, geometry.Indices.GetCount());
					ZE_DRAW_TAG_END(renderData.CL);
				}

				ZE_DRAW_TAG_END(renderData.CL);
				renderData.CL.Close(renderData.Dev);
				renderData.Dev.ExecuteMain(renderData.CL);
			}
		}
	}
}