#include "GFX/Pipeline/RenderPass/DirectionalLight.h"
#include "GFX/Resource/Constant.h"
#include "Data/Tags.h"

namespace ZE::GFX::Pipeline::RenderPass::DirectionalLight
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 3, "Incorrect size for DirectionalLight initialization formats!");
		return Initialize(dev, buildData, formats.at(0), formats.at(1), formats.at(2));
	}

	PassDesc GetDesc(PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth) noexcept
	{
		PassDesc desc{ Base(CorePassType::DirectionalLight) };
		desc.InitializeFormats.reserve(3);
		desc.InitializeFormats.emplace_back(formatLighting);
		desc.InitializeFormats.emplace_back(formatShadow);
		desc.InitializeFormats.emplace_back(formatShadowDepth);
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
		execData->State.Free(dev);
		delete execData;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData,
		PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(Float3), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light direction
		desc.AddRange({ 1, 1, 4, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Directional light buffer
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Shadow map
		desc.AddRange({ 4, 1, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // GBuff
		desc.AddRange(buildData.DynamicDataRange, Resource::ShaderType::Pixel);
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		const auto& schema = buildData.BindingLib.GetSchema(passData->BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, "DirectionalLightPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Blender = Resource::BlendType::Light;
		psoDesc.SetDepthClip(false);
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatLighting;
		ZE_PSO_SET_NAME(psoDesc, "DirectionalLight");
		passData->State.Init(dev, psoDesc, schema);

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Directional Light");

		auto group = Data::GetDirectionalLightGroup();
		if (group.size())
		{
			ZE_PERF_GUARD("Directional Light - light present");
			Resources ids = *passData.Resources.CastConst<Resources>();
			ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

			ZE_DRAW_TAG_BEGIN(dev, cl, "Directional Light", Pixel(0xF5, 0xF5, 0xD1));

			renderData.Buffers.ClearRTV(cl, ids.ShadowMap, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
			renderData.Buffers.ClearDSV(cl, ids.ShadowMapDepth, 0.0f, 0);
			renderData.Buffers.Barrier(cl, BarrierTransition{ ids.ShadowMap, TextureLayout::RenderTarget, TextureLayout::ShaderResource,
				Base(ResourceAccess::RenderTarget), Base(ResourceAccess::ShaderResource), Base(StageSync::RenderTarget), Base(StageSync::PixelShading) });

			renderData.Buffers.BeginRaster(cl, ids.Lighting);
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
			ctx.BindingSchema.SetGraphics(cl);
			data.State.Bind(cl);

			ctx.SetFromEnd(3);
			renderData.Buffers.SetSRV(cl, ctx, ids.ShadowMap);
			renderData.Buffers.SetSRV(cl, ctx, ids.GBufferDepth);
			renderData.BindRendererDynamicData(cl, ctx);
			renderData.SettingsBuffer.Bind(cl, ctx);
			ctx.Reset();

			ZE_PERF_START("Directional Light - main loop");
			for (EID entity : group)
			{
				ZE_PERF_GUARD("Directional Light - single loop item");
				Resource::Constant<Float3> direction(dev, group.get<Data::Direction>(entity).Direction);
				direction.Bind(cl, ctx);
				group.get<Data::DirectionalLightBuffer>(entity).Buffer.Bind(cl, ctx);
				ctx.Reset();

				cl.DrawFullscreen(dev);
			}
			ZE_PERF_STOP();

			renderData.Buffers.Barrier(cl, BarrierTransition{ ids.ShadowMap, TextureLayout::ShaderResource, TextureLayout::RenderTarget,
				Base(ResourceAccess::ShaderResource), Base(ResourceAccess::RenderTarget), Base(StageSync::PixelShading), Base(StageSync::RenderTarget) });
			renderData.Buffers.EndRaster(cl);
			ZE_DRAW_TAG_END(dev, cl);
		}
	}
}