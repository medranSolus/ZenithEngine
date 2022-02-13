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

		const auto& schema = buildData.BindingLib.GetSchema(passData->BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"PhongDepthVS", buildData.ShaderCache);
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Pos3D);
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Normal);
		psoDesc.InputLayout.emplace_back(Resource::InputParam::TexCoord);
		psoDesc.InputLayout.emplace_back(Resource::InputParam::Bitangent);
		ZE_PSO_SET_NAME(psoDesc, "LambertianDepth");
		passData->StateDepth.Init(dev, psoDesc, schema);

		psoDesc.SetShader(psoDesc.VS, L"PhongVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"PhongPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthBefore;
		psoDesc.RenderTargetsCount = 3;
		psoDesc.FormatsRT[0] = formatColor;
		psoDesc.FormatsRT[1] = formatNormal;
		psoDesc.FormatsRT[2] = formatSpecular;
		ZE_PSO_SET_NAME(psoDesc, "Lambertian");
		passData->StateNormal.Init(dev, psoDesc, schema);

		passData->TransformBuffers.emplace_back(dev, nullptr, SINGLE_BUFFER_SIZE, true);

		return passData;
	}

	void Execute(RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		Data& data = *reinterpret_cast<Data*>(passData.OptData);

		// Clearing data on first usage
		renderData.CL.Open(renderData.Dev);
		ZE_DRAW_TAG_BEGIN(renderData.CL, L"Lambertian Clear", PixelVal::White);

		renderData.Buffers.ClearDSV(renderData.CL, ids.DepthStencil, 1.0f, 0);
		renderData.Buffers.ClearRTV(renderData.CL, ids.Color, ColorF4());
		renderData.Buffers.ClearRTV(renderData.CL, ids.Normal, ColorF4());
		renderData.Buffers.ClearRTV(renderData.CL, ids.Specular, ColorF4());

		ZE_DRAW_TAG_END(renderData.CL);
		renderData.CL.Close(renderData.Dev);
		renderData.Dev.ExecuteMain(renderData.CL);

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

			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			const auto& transforms = data.World.ActiveScene->TransformsGlobal;
			const auto& geometries = data.World.ActiveScene->Geometries;
			Float3 cameraPos = data.World.ActiveScene->Cameras[data.World.ActiveScene->CameraPositions.at(data.World.CurrnetCamera)].Position;

			// Depth pre-pass
			// Send data in batches to fill every transform buffer to it's maximal capacity (64KB)
			for (U64 i = 0, j = 0; i < data.World.MeshesInfo.Size;)
			{
				renderData.CL.Open(renderData.Dev, data.StateDepth);
				ZE_DRAW_TAG_BEGIN(renderData.CL, L"Lambertian Depth", Pixel(0xC2, 0xC5, 0xCC));
				ctx.BindingSchema.SetGraphics(renderData.CL);
				renderData.Buffers.SetDSV(renderData.CL, ids.DepthStencil);

				ctx.SetFromEnd(1);
				auto& cbuffer = data.TransformBuffers.at(j);
				cbuffer.Bind(renderData.CL, ctx);
				ctx.Reset();

				// Compute single batch
				TransformCBuffer* buffer = reinterpret_cast<TransformCBuffer*>(cbuffer.GetRegion());
				buffer->CameraPos = cameraPos;
				for (U32 k = 0; k < TRANSFORM_COUNT && i < data.World.MeshesInfo.Size; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(renderData.CL, std::to_wstring(k).c_str(), PixelVal::Gray);
					const auto& info = data.World.Meshes[i];

					const auto& transform = transforms[info.TransformIndex];
					Matrix matrix = Math::XMMatrixScalingFromVector(Math::XMLoadFloat3(&transform.Scale)) *
						Math::XMMatrixRotationQuaternion(Math::XMLoadFloat4(&transform.Rotation)) *
						Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&transform.Position));

					buffer->Transforms[k].Model = Math::XMMatrixTranspose(matrix);
					buffer->Transforms[k].ModelViewProjection = Math::XMMatrixTranspose(matrix * Math::XMMatrixIdentity() * Math::XMMatrixIdentity());

					Resource::Constant<U32> meshBatchId(renderData.Dev, k);
					meshBatchId.Bind(renderData.CL, ctx);
					ctx.Reset();

					const auto& geometry = geometries[info.GeometryIndex];
					geometry.Vertices.Bind(renderData.CL);
					geometry.Indices.Bind(renderData.CL);

					renderData.CL.Draw(renderData.Dev, geometry.Indices.GetCount());
					ZE_DRAW_TAG_END(renderData.CL);
				}

				ZE_DRAW_TAG_END(renderData.CL);
				renderData.CL.Close(renderData.Dev);
				renderData.Dev.ExecuteMain(renderData.CL);
			}

			const auto& materials = data.World.ActiveScene->Materials;
			U64 currentMaterialIndex = UINT64_MAX;
			// Normal pass
			for (U64 i = 0, j = 0; i < data.World.MeshesInfo.Size;)
			{
				renderData.CL.Open(renderData.Dev, data.StateNormal);
				ZE_DRAW_TAG_BEGIN(renderData.CL, L"Lambertian", Pixel(0xC2, 0xC5, 0xCC));
				ctx.BindingSchema.SetGraphics(renderData.CL);
				renderData.Buffers.SetOutput<3>(renderData.CL, &ids.Color, ids.DepthStencil);

				ctx.SetFromEnd(1);
				data.TransformBuffers.at(j).Bind(renderData.CL, ctx);
				renderData.EngineData.Bind(renderData.CL, ctx);
				ctx.Reset();

				// Compute single batch
				for (U32 k = 0; k < TRANSFORM_COUNT && i < data.World.MeshesInfo.Size; ++k, ++i)
				{
					ZE_DRAW_TAG_BEGIN(renderData.CL, std::to_wstring(k).c_str(), Pixel(0xAD, 0xAD, 0xC9));
					Resource::Constant<U32> meshBatchId(renderData.Dev, k);
					meshBatchId.Bind(renderData.CL, ctx);

					const auto& info = data.World.Meshes[i];
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
					ZE_DRAW_TAG_END(renderData.CL);
				}

				ZE_DRAW_TAG_END(renderData.CL);
				renderData.CL.Close(renderData.Dev);
				renderData.Dev.ExecuteMain(renderData.CL);
			}
		}
	}
}