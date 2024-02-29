#include "GFX/Pipeline/RenderPass/DirectionalLight.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::DirectionalLight
{
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData,
		PixelFormat formatLighting, PixelFormat formatShadow, PixelFormat formatShadowDepth)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(Float3), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Light direction
		desc.AddRange({ 1, 1, 4, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV }); // Directional light buffer
		desc.AddRange({ 1, 0, 3, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Shadow map
		desc.AddRange({ 4, 1, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // GBuff
		desc.AddRange({ 1, 12, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer }); // Renderer dynamic data
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
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

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Directional Light");
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);
		cl.Open(dev, data.State);

		// Clearing data on first usage
		ZE_DRAW_TAG_BEGIN(dev, cl, "Lighting Clear", PixelVal::White);
		renderData.Buffers.ClearRTV(cl, ids.Lighting, ColorF4(0.0f, 0.0f, 0.0f, 0.0f));
		ZE_DRAW_TAG_END(dev, cl);

		auto group = Data::GetDirectionalLightGroup();
		if (group.size())
		{
			ZE_PERF_GUARD("Directional Light - light present");
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			ZE_DRAW_TAG_BEGIN(dev, cl, "Directional Light", Pixel(0xF5, 0xF5, 0xD1));
			renderData.Buffers.ClearRTV(cl, ids.ShadowMap, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
			renderData.Buffers.ClearDSV(cl, ids.ShadowMapDepth, 0.0f, 0);
			renderData.Buffers.BarrierTransition(cl, ids.ShadowMap, Resource::StateRenderTarget, Resource::StateShaderResourcePS);

			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetRTV(cl, ids.Lighting);

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
			renderData.Buffers.BarrierTransition(cl, ids.ShadowMap, Resource::StateShaderResourcePS, Resource::StateRenderTarget);
			ZE_DRAW_TAG_END(dev, cl);
		}
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}