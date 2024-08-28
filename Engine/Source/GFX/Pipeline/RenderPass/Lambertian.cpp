#include "GFX/Pipeline/RenderPass/Lambertian.h"
#include "GFX/Pipeline/RenderPass/Utils.h"
#include "GFX/Vertex.h"

namespace ZE::GFX::Pipeline::RenderPass::Lambertian
{
	static bool Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats)
	{
		ZE_ASSERT(formats.size() == 6, "Incorrect size for Lambertian initialization formats!");
		Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData), formats.at(0), formats.at(1),
			formats.at(2), formats.at(3), formats.at(4), formats.at(5));
		return false;
	}

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 6, "Incorrect size for Lambertian initialization formats!");
		return Initialize(dev, buildData, formats.at(0), formats.at(1),
			formats.at(2), formats.at(3), formats.at(4), formats.at(5));
	}

	PassDesc GetDesc(PixelFormat formatDS, PixelFormat formatNormal, PixelFormat formatAlbedo,
		PixelFormat formatMaterialParams, PixelFormat formatMotion, PixelFormat formatReactive) noexcept
	{
		PassDesc desc{ static_cast<PassType>(CorePassType::Lambertian) };
		desc.InitializeFormats.reserve(6);
		desc.InitializeFormats.emplace_back(formatDS);
		desc.InitializeFormats.emplace_back(formatNormal);
		desc.InitializeFormats.emplace_back(formatAlbedo);
		desc.InitializeFormats.emplace_back(formatMaterialParams);
		desc.InitializeFormats.emplace_back(formatMotion);
		desc.InitializeFormats.emplace_back(formatReactive);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		return desc;
	}

	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->StateDepth.Free(dev);
		U8 stateCount = Data::MaterialPBR::GetLastPipelineStateNumber() + 1;
		while (stateCount--)
		{
			execData->StatesSolid[stateCount].Free(dev);
			execData->StatesTransparent[stateCount].Free(dev);
		}
		execData->StatesSolid.DeleteArray();
		execData->StatesTransparent.DeleteArray();
		delete execData;
	}

	void Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat formatDS, PixelFormat formatNormal,
		PixelFormat formatAlbedo, PixelFormat formatMaterialParams, PixelFormat formatMotion, PixelFormat formatReactive)
	{
		const bool isMotion = Settings::ComputeMotionVectors();
		const bool isReactive = Settings::GetUpscaler() == UpscalerType::Fsr2 || Settings::GetUpscaler() == UpscalerType::XeSS;
		if (isMotion != passData.MotionEnabled || isReactive != passData.ReactiveEnabled)
		{
			passData.MotionEnabled = isMotion;
			passData.ReactiveEnabled = isReactive;

			const auto& schema = buildData.BindingLib.GetSchema(passData.BindingIndex);

			Resource::PipelineStateDesc psoDesc;
			psoDesc.FormatDS = formatDS;
			psoDesc.InputLayout = Vertex::GetLayout();
			psoDesc.SetShader(dev, psoDesc.VS, isMotion ? "LambertVS_M" : "LambertVS", buildData.ShaderCache);
			psoDesc.RenderTargetsCount = 3 + isMotion + isReactive;
			psoDesc.FormatsRT[0] = formatNormal;
			psoDesc.FormatsRT[1] = formatAlbedo;
			psoDesc.FormatsRT[2] = formatMaterialParams;
			psoDesc.FormatsRT[3] = isMotion ? formatMotion : formatReactive;
			psoDesc.FormatsRT[4] = formatReactive;

			std::string shaderName = "PbrPS";
			if (isMotion || isReactive)
			{
				shaderName += "_";
				if (isMotion)
					shaderName += "M";
				if (isReactive)
					shaderName += "R";
			}
			U8 stateIndex = Data::MaterialPBR::GetLastPipelineStateNumber() + 1;
			while (stateIndex--)
			{
				std::string suffix = Data::MaterialPBR::DecodeShaderSuffix(Data::MaterialPBR::GetShaderFlagsForState(stateIndex));
				if ((isMotion || isReactive) && suffix.size())
					suffix.erase(suffix.begin());

				psoDesc.SetShader(dev, psoDesc.PS, shaderName + suffix, buildData.ShaderCache);

				psoDesc.DepthStencil = Resource::DepthStencilMode::DepthBefore;
				ZE_PSO_SET_NAME(psoDesc, "LambertianSolid" + suffix);
				passData.StatesSolid[stateIndex].Free(dev);
				passData.StatesSolid[stateIndex].Init(dev, psoDesc, schema);

				psoDesc.DepthStencil = Resource::DepthStencilMode::StencilOff;
				ZE_PSO_SET_NAME(psoDesc, "LambertianTransparent" + suffix);
				passData.StatesTransparent[stateIndex].Free(dev);
				passData.StatesTransparent[stateIndex].Init(dev, psoDesc, schema);
			}
		}
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatDS, PixelFormat formatNormal,
		PixelFormat formatAlbedo, PixelFormat formatMaterialParams, PixelFormat formatMotion, PixelFormat formatReactive)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Vertex, Binding::RangeFlag::CBV }); // Transform buffer
		desc.AddRange({ 1, 0, 4, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // MaterialPBR buffer
		desc.AddRange({ 4, 0, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Texture, normal, specular, parallax
		desc.AddRange(buildData.DynamicDataRange, Resource::ShaderType::Vertex | Resource::ShaderType::Pixel);
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Vertex | Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		U8 stateIndex = Data::MaterialPBR::GetLastPipelineStateNumber() + 1;
		passData->StatesSolid = new Resource::PipelineStateGfx[stateIndex];
		passData->StatesTransparent = new Resource::PipelineStateGfx[stateIndex];
		passData->MotionEnabled = !Settings::ComputeMotionVectors();
		passData->ReactiveEnabled = Settings::GetUpscaler() != UpscalerType::Fsr2 && Settings::GetUpscaler() != UpscalerType::XeSS;
		Update(dev, buildData, *passData, formatDS, formatNormal, formatAlbedo, formatMaterialParams, formatMotion, formatReactive);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "LambertDepthVS", buildData.ShaderCache);
		psoDesc.FormatDS = formatDS;
		psoDesc.InputLayout = Vertex::GetLayout();
		ZE_PSO_SET_NAME(psoDesc, "LambertianDepth");
		passData->StateDepth.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Lambertian");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_ASSERT(data.MotionEnabled == Settings::ComputeMotionVectors(),
			"Lambertian pass not updated for changed motion vectors output settings!");
		ZE_ASSERT(data.ReactiveEnabled == (Settings::GetUpscaler() == UpscalerType::Fsr2 || Settings::GetUpscaler() == UpscalerType::XeSS),
			"Lambertian pass not updated for changed reactive mask output settings!");

		const Matrix viewProjection = Math::XMLoadFloat4x4(&renderData.DynamicData.ViewProjectionTps);
		const Matrix prevViewProjection = Math::XMLoadFloat4x4(&renderData.GraphData.PrevViewProjectionTps);
		const Vector cameraPos = Math::XMLoadFloat3(&renderData.DynamicData.CameraPos);

		// Compute visibility of objects inside camera view
		ZE_PERF_START("Lambertian - frustum culling");
		Math::BoundingFrustum frustum = Data::GetFrustum(Math::XMLoadFloat4x4(&renderData.GraphData.Projection), Settings::MaxRenderDistance);
		frustum.Transform(frustum, 1.0f, Math::XMLoadFloat4(&Settings::Data.get<Data::TransformGlobal>(renderData.GraphData.CurrentCamera).Rotation), cameraPos);
		Utils::FrustumCulling<InsideFrustumSolid, InsideFrustumNotSolid>(Data::GetRenderGroup<Data::RenderLambertian>(), frustum);
		ZE_PERF_STOP();

		// Use new group visible only in current frustum and sort
		auto solidGroup = Data::GetVisibleRenderGroup<Data::RenderLambertian, InsideFrustumSolid>();
		auto transparentGroup = Data::GetVisibleRenderGroup<Data::RenderLambertian, InsideFrustumNotSolid>();
		const U64 solidCount = solidGroup.size();
		const U64 transparentCount = transparentGroup.size();

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		auto& cbuffer = *renderData.DynamicBuffer;

		EID currentMaterial = INVALID_EID;
		U8 currentState = UINT8_MAX;
		if (solidCount)
		{
			ZE_PERF_GUARD("Lambertian - solid present");

			ZE_PERF_START("Lambertian - solid view sorting");
			Utils::ViewSortAscending(solidGroup, cameraPos);
			ZE_PERF_STOP();

			// Depth pre-pass
			ZE_PERF_START("Lambertian Depth");
			data.StateDepth.Bind(cl);
			ZE_DRAW_TAG_BEGIN(dev, cl, "Lambertian Depth", Pixel(0xC2, 0xC5, 0xCC));
			ctx.BindingSchema.SetGraphics(cl);
			//renderData.Buffers.SetDSV(cl, ids.DepthStencil);

			ZE_PERF_START("Lambertian Depth - main loop");
			for (U64 i = 0; i < solidCount; ++i)
			{
				ZE_PERF_GUARD("Lambertian Depth - single loop item");
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), PixelVal::Gray);

				EID entity = solidGroup[i];
				const auto& transform = solidGroup.get<Data::TransformGlobal>(entity);

				Matrix m = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
				Matrix mvp = viewProjection * m;
				Resource::DynamicBufferAlloc transformAlloc;
				if (Settings::ComputeMotionVectors())
				{
					const auto& transformPrev = Settings::Data.get<Data::TransformPrevious>(entity);

					ModelTransformBufferMotion transformBuffer;
					Math::XMStoreFloat4x4(&transformBuffer.ModelTps, m);
					Math::XMStoreFloat4x4(&transformBuffer.ModelViewProjectionTps, mvp);
					Math::XMStoreFloat4x4(&transformBuffer.PrevModelViewProjectionTps,
						prevViewProjection * Math::XMMatrixTranspose(Math::GetTransform(transformPrev.Position, transformPrev.Rotation, transformPrev.Scale)));

					transformAlloc = cbuffer.Alloc(dev, &transformBuffer, sizeof(ModelTransformBufferMotion));
				}
				else
				{
					ModelTransformBuffer transformBuffer;
					Math::XMStoreFloat4x4(&transformBuffer.ModelTps, m);
					Math::XMStoreFloat4x4(&transformBuffer.ModelViewProjectionTps, mvp);

					transformAlloc = cbuffer.Alloc(dev, &transformBuffer, sizeof(ModelTransformBuffer));
				}

				auto& transformInfo = solidGroup.get<InsideFrustumSolid>(entity);
				transformInfo.Transform = transformAlloc;
				cbuffer.Bind(cl, ctx, transformAlloc);
				ctx.Reset();

				Settings::Data.get<Resource::Mesh>(solidGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
				ZE_DRAW_TAG_END(dev, cl);
			}
			ZE_PERF_STOP();

			ZE_DRAW_TAG_END(dev, cl);
			ZE_PERF_STOP();

			// Sort by pipeline state
			ZE_PERF_START("Lambertian - solid material sort");
			solidGroup.sort<Data::MaterialID>([&](const auto& m1, const auto& m2) -> bool
				{
					const U8 state1 = Data::MaterialPBR::GetPipelineStateNumber(Settings::Data.get<Data::PBRFlags>(m1.ID));
					const U8 state2 = Data::MaterialPBR::GetPipelineStateNumber(Settings::Data.get<Data::PBRFlags>(m2.ID));
					return state1 < state2;
				});
			currentState = Data::MaterialPBR::GetPipelineStateNumber(Settings::Data.get<Data::PBRFlags>(solidGroup.get<Data::MaterialID>(solidGroup[0]).ID));
			ZE_PERF_STOP();

			// Solid pass
			ZE_PERF_START("Lambertian Solid");
			data.StatesSolid[currentState].Bind(cl);
			ZE_DRAW_TAG_BEGIN(dev, cl, "Lambertian Solid", Pixel(0xC2, 0xC5, 0xCC));
			ctx.BindingSchema.SetGraphics(cl);
			/*
			if (isMotion || isReactive)
				renderData.Buffers.SetOutputSparse(cl, &ids.Normal, ids.DepthStencil, 3 + isMotion + isReactive);
			else
				renderData.Buffers.SetOutput<3>(cl, &ids.Normal, ids.DepthStencil, true);
			*/
			ctx.SetFromEnd(1);
			renderData.BindRendererDynamicData(cl, ctx);
			renderData.SettingsBuffer.Bind(cl, ctx);
			ctx.Reset();

			ZE_PERF_START("Lambertian Solid - main loop");
			for (U64 i = 0; i < solidCount; ++i)
			{
				ZE_PERF_GUARD("Lambertian Solid - single loop item");
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0xAD, 0xAD, 0xC9));

				EID entity = solidGroup[i];
				cbuffer.Bind(cl, ctx, solidGroup.get<InsideFrustumSolid>(entity).Transform);

				const Data::MaterialID material = solidGroup.get<Data::MaterialID>(entity);
				if (currentMaterial != material.ID)
				{
					currentMaterial = material.ID;

					const auto& buffers = Settings::Data.get<Data::MaterialBuffersPBR>(currentMaterial);
					buffers.BindBuffer(cl, ctx);
					buffers.BindTextures(cl, ctx);

					const U8 state = Data::MaterialPBR::GetPipelineStateNumber(Settings::Data.get<Data::PBRFlags>(currentMaterial));
					if (currentState != state)
					{
						currentState = state;
						data.StatesSolid[state].Bind(cl);
					}
				}
				ctx.Reset();

				Settings::Data.get<Resource::Mesh>(solidGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
				ZE_DRAW_TAG_END(dev, cl);
			}
			ZE_PERF_STOP();

			ZE_DRAW_TAG_END(dev, cl);
			ZE_PERF_STOP();

			currentMaterial = INVALID_EID;
			currentState = UINT8_MAX;
		}

		// Transparent pass
		if (transparentCount)
		{
			ZE_PERF_GUARD("Lambertian - transparent present");

			ZE_PERF_START("Lambertian - transparent view sorting");
			Utils::ViewSortDescending(transparentGroup, cameraPos);
			ZE_PERF_STOP();

			ZE_PERF_START("Lambertian Transparent");
			ZE_DRAW_TAG_BEGIN(dev, cl, "Lambertian Transparent", Pixel(0xEC, 0xED, 0xEF));
			ctx.BindingSchema.SetGraphics(cl);
			/*
			if (isMotion || isReactive)
				renderData.Buffers.SetOutputSparse(cl, &ids.Normal, ids.DepthStencil, 3 + isMotion + isReactive);
			else
				renderData.Buffers.SetOutput<3>(cl, &ids.Normal, ids.DepthStencil, true);
			*/
			ctx.SetFromEnd(1);
			renderData.BindRendererDynamicData(cl, ctx);
			renderData.SettingsBuffer.Bind(cl, ctx);
			ctx.Reset();

			ZE_PERF_START("Lambertian Transparent - main loop");
			for (U64 i = 0; i < transparentCount; ++i)
			{
				ZE_PERF_GUARD("Lambertian Transparent - single loop item");
				ZE_DRAW_TAG_BEGIN(dev, cl, ("Mesh_" + std::to_string(i)).c_str(), Pixel(0xD6, 0xD6, 0xE4));

				EID entity = transparentGroup[i];
				const auto& transform = transparentGroup.get<Data::TransformGlobal>(entity);

				Matrix m = Math::XMMatrixTranspose(Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));
				Matrix mvp = viewProjection * m;
				if (Settings::ComputeMotionVectors())
				{
					const auto& transformPrev = Settings::Data.get<Data::TransformPrevious>(entity);

					ModelTransformBufferMotion transformBuffer;
					Math::XMStoreFloat4x4(&transformBuffer.ModelTps, m);
					Math::XMStoreFloat4x4(&transformBuffer.ModelViewProjectionTps, mvp);
					Math::XMStoreFloat4x4(&transformBuffer.PrevModelViewProjectionTps,
						prevViewProjection * Math::XMMatrixTranspose(Math::GetTransform(transformPrev.Position, transformPrev.Rotation, transformPrev.Scale)));

					cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(ModelTransformBufferMotion));
				}
				else
				{
					ModelTransformBuffer transformBuffer;
					Math::XMStoreFloat4x4(&transformBuffer.ModelTps, m);
					Math::XMStoreFloat4x4(&transformBuffer.ModelViewProjectionTps, mvp);

					cbuffer.AllocBind(dev, cl, ctx, &transformBuffer, sizeof(ModelTransformBuffer));
				}

				const Data::MaterialID material = transparentGroup.get<Data::MaterialID>(entity);
				if (currentMaterial != material.ID)
				{
					currentMaterial = material.ID;

					const auto& buffers = Settings::Data.get<Data::MaterialBuffersPBR>(material.ID);
					buffers.BindBuffer(cl, ctx);
					buffers.BindTextures(cl, ctx);

					const U8 state = Data::MaterialPBR::GetPipelineStateNumber(Settings::Data.get<Data::PBRFlags>(currentMaterial));
					if (currentState != state)
					{
						currentState = state;
						data.StatesTransparent[state].Bind(cl);
					}
				}
				ctx.Reset();

				Settings::Data.get<Resource::Mesh>(transparentGroup.get<Data::MeshID>(entity).ID).Draw(dev, cl);
				ZE_DRAW_TAG_END(dev, cl);
			}
			ZE_PERF_STOP();

			ZE_DRAW_TAG_END(dev, cl);
			ZE_PERF_STOP();
		}

		ZE_PERF_START("Lambertian - visibility clear");
		// Remove current visibility
		Settings::Data.clear<InsideFrustumSolid, InsideFrustumNotSolid>();
		ZE_PERF_STOP();
	}
}