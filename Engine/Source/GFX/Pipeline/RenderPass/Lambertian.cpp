#include "GFX/Pipeline/RenderPass/Lambertian.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::Lambertian
{
	Data* Setup(Device& dev, RendererBuildData& buildData, WorldInfo& worldData, PixelFormat formatDS,
		PixelFormat formatColor, PixelFormat formatNormal, PixelFormat formatSpecular)
	{
		Data* passData = new Data{ worldData };

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(U32), 0, Resource::ShaderType::Vertex, Binding::RangeFlag::Constant });
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
		desc.AddRange({ 4, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 1, 1, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"PhongVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"PhongPS", buildData.ShaderCache);
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.RenderTargetsCount = 3;
		psoDesc.FormatsRT[0] = formatColor;
		psoDesc.FormatsRT[1] = formatNormal;
		psoDesc.FormatsRT[2] = formatSpecular;
		psoDesc.FormatDS = formatDS;
		ZE_PSO_SET_NAME(psoDesc, "Lambertian");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		passData->TransformBuffers.emplace_back(dev, nullptr, SINGLE_BUFFER_SIZE, true);

		return passData;
	}

	void Execute(RendererExecuteData& renderData, PassData& passData)
	{
		Data& data = *reinterpret_cast<Data*>(passData.OptData);

		if (data.World.MeshesInfo.Size)
		{
			// Resize temporary buffer for transform data
			U64 buffCount = Math::DivideRoundUp(data.World.MeshesInfo.Size * sizeof(TransformCBuffer), static_cast<U64>(SINGLE_BUFFER_SIZE)) / SINGLE_BUFFER_SIZE;
			if (buffCount + BUFFER_SHRINK_STEP < data.TransformBuffers.size())
			{
				for (U64 i = buffCount; i < data.TransformBuffers.size(); ++i)
					data.TransformBuffers.at(i).Free(renderData.Dev);
				data.TransformBuffers.resize(buffCount);
			}
			else if (buffCount > data.TransformBuffers.size())
			{
				U64 i = data.TransformBuffers.size();
				data.TransformBuffers.resize(buffCount);
				for (; i < buffCount; ++i)
					data.TransformBuffers.at(i).Init(renderData.Dev, nullptr, SINGLE_BUFFER_SIZE, true);
			}

			Resources ids = *passData.Buffers.CastConst<Resources>();
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			const auto& transforms = data.World.ActiveScene->TransformsGlobal;
			const auto& materials = data.World.ActiveScene->Materials;
			const auto& geometries = data.World.ActiveScene->Geometries;
			U64 currentMaterialIndex = UINT64_MAX;
			Float3 cameraPos = { 0.0, 0.0f, 0.0f };

			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			for (U64 i = 0, j = 0; i < data.World.MeshesInfo.Size;)
			{
				renderData.CL.Open(renderData.Dev, data.State);
				ctx.BindingSchema.SetGraphics(renderData.CL);
				renderData.Buffers.SetOutput<3>(renderData.CL, &ids.Color, ids.DepthStencil);

				auto& cbuffer = data.TransformBuffers.at(j);
				ctx.SetFromEnd(1);
				renderData.EngineData.Bind(renderData.CL, ctx);
				cbuffer.Bind(renderData.CL, ctx);
				ctx.Reset();

				// Compute single batch
				TransformCBuffer* buffer = reinterpret_cast<TransformCBuffer*>(cbuffer.GetRegion());
				buffer->CameraPos = cameraPos;
				for (U32 k = 0; k < SINGLE_BUFFER_SIZE / sizeof(TransformCBuffer) && i < data.World.MeshesInfo.Size; ++k, ++i)
				{
					const auto& info = data.World.Meshes[i];

					const auto& transform = transforms[info.TransformIndex];
					Matrix matrix = Math::XMMatrixScalingFromVector(Math::XMLoadFloat3(&transform.Scale)) *
						Math::XMMatrixRotationQuaternion(Math::XMLoadFloat4(&transform.Rotation)) *
						Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&transform.Position));

					buffer->Transforms[k].Model = Math::XMMatrixTranspose(matrix);
					buffer->Transforms[k].ModelViewProjection = Math::XMMatrixTranspose(matrix * Math::XMMatrixIdentity() * Math::XMMatrixIdentity());

					Resource::Constant<U32> meshBatchId(renderData.Dev, k);
					meshBatchId.Bind(renderData.CL, ctx);

					if (currentMaterialIndex != info.MaterialIndex)
					{
						currentMaterialIndex = info.MaterialIndex;
						const auto& material = materials[currentMaterialIndex];
						material.BindBuffer(renderData.CL, ctx);
						material.BindTextures(renderData.CL, ctx);
					}
					ctx.Reset();

					const auto& geometry = geometries[info.GeometryIndex];
					geometry.Vertices.Bind(renderData.CL);
					geometry.Indices.Bind(renderData.CL);

					renderData.CL.Draw(renderData.Dev, geometry.Indices.GetCount());
				}

				renderData.CL.Close(renderData.Dev);
				renderData.Dev.ExecuteMain(renderData.CL);
			}
		}
	}
}