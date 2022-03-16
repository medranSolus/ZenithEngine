#include "GFX/Pipeline/RenderPass/DirectionalLight.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::DirectionalLight
{
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData,
		PixelFormat formatColor, PixelFormat formatSpecular,
		PixelFormat formatShadow, PixelFormat formatShadowDepth)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(Float3), 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant });
		desc.AddRange({ 1, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
		desc.AddRange({ 1, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 3, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 1, 12, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Pixel);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		const auto& schema = buildData.BindingLib.GetSchema(passData->BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(psoDesc.VS, L"FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(psoDesc.PS, L"DirectionalLightPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Blender = Resource::BlendType::Light;
		psoDesc.Culling = Resource::CullMode::Front;
		psoDesc.SetDepthClip(false);
		psoDesc.RenderTargetsCount = 2;
		psoDesc.FormatsRT[0] = formatColor;
		psoDesc.FormatsRT[1] = formatSpecular;
		ZE_PSO_SET_NAME(psoDesc, "DirectionalLight");
		passData->State.Init(dev, psoDesc, schema);

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);
		cl.Open(dev);

		// Clearing data on first usage
		ZE_DRAW_TAG_BEGIN(cl, L"Lighting Clear", PixelVal::White);
		renderData.Buffers.ClearRTV(cl, ids.Color, ColorF4(0.0f, 0.0f, 0.0f, 0.0f));
		renderData.Buffers.ClearRTV(cl, ids.Specular, ColorF4(0.0f, 0.0f, 0.0f, 0.0f));
		ZE_DRAW_TAG_END(cl);

		auto group = Data::GetDirectionalLightGroup(renderData.Registry);
		if (group.size())
		{
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			ZE_DRAW_TAG_BEGIN(cl, L"Directional Light", Pixel(0xF5, 0xF5, 0xD1));

			renderData.Buffers.ClearRTV(cl, ids.ShadowMap, ColorF4());
			renderData.Buffers.ClearDSV(cl, ids.ShadowMapDepth, 1.0f, 0);

			ctx.BindingSchema.SetGraphics(cl);
			renderData.Buffers.SetRTV<2>(cl, &ids.Color, true);

			ctx.SetFromEnd(3);
			renderData.Buffers.SetSRV(cl, ctx, ids.ShadowMap);
			renderData.Buffers.SetSRV(cl, ctx, ids.GBufferNormal);
			renderData.DynamicBuffer.Bind(cl, ctx);
			renderData.SettingsBuffer.Bind(cl, ctx);
			ctx.Reset();

			for (auto entity : group)
			{
				Resource::Constant<Float3> direction(dev, group.get<Data::Direction>(entity).Direction);
				direction.Bind(cl, ctx);
				group.get<Data::DirectionalLightBuffer>(entity).Buffer.Bind(cl, ctx);
				ctx.Reset();

				cl.DrawFullscreen(dev);
			}
			ZE_DRAW_TAG_END(cl);
		}

		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}