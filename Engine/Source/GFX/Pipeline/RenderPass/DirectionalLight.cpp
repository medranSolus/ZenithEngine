#include "GFX/Pipeline/RenderPass/DirectionalLight.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::DirectionalLight
{
	Data* Setup(Device& dev, RendererBuildData& buildData, Info::World& worldData,
		PixelFormat formatColor, PixelFormat formatSpecular,
		PixelFormat formatShadow, PixelFormat formatShadowDepth)
	{
		Data* passData = new Data{ worldData };

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

	void Execute(RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		Data& data = *reinterpret_cast<Data*>(passData.OptData);
		renderData.CL.Open(renderData.Dev);

		// Clearing data on first usage
		ZE_DRAW_TAG_BEGIN(renderData.CL, L"Lighting Clear", PixelVal::White);
		renderData.Buffers.ClearRTV(renderData.CL, ids.Color, ColorF4());
		renderData.Buffers.ClearRTV(renderData.CL, ids.Specular, ColorF4());
		ZE_DRAW_TAG_END(renderData.CL);

		if (data.World.DirectionalLightInfo.Size)
		{
			const auto& directions = data.World.ActiveScene->DirectionalLightDirections;
			const auto& lights = data.World.ActiveScene->DirectionalLightBuffers;
			Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

			ZE_DRAW_TAG_BEGIN(renderData.CL, L"Directional Light", Pixel(0xF5, 0xF5, 0xD1));

			renderData.Buffers.ClearRTV(renderData.CL, ids.ShadowMap, ColorF4());
			renderData.Buffers.ClearDSV(renderData.CL, ids.ShadowMapDepth, 1.0f, 0);

			ctx.BindingSchema.SetGraphics(renderData.CL);
			renderData.Buffers.SetRTV<2>(renderData.CL, &ids.Color, true);

			ctx.SetFromEnd(3);
			renderData.Buffers.SetSRV(renderData.CL, ctx, ids.ShadowMap);
			renderData.Buffers.SetSRV(renderData.CL, ctx, ids.GBufferNormal);
			data.World.DynamicDataBuffer.Bind(renderData.CL, ctx);
			renderData.EngineData.Bind(renderData.CL, ctx);
			ctx.Reset();

			for (U64 i = 0; i < data.World.DirectionalLightInfo.Size; ++i)
			{
				Resource::Constant<Float3> direction(renderData.Dev, directions[i]);
				direction.Bind(renderData.CL, ctx);
				lights[i].Bind(renderData.CL, ctx);
				ctx.Reset();

				renderData.CL.DrawFullscreen(renderData.Dev);
			}
			ZE_DRAW_TAG_END(renderData.CL);
		}

		renderData.CL.Close(renderData.Dev);
		renderData.Dev.ExecuteMain(renderData.CL);
	}
}