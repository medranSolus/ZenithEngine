#include "GFX/Pipeline/RenderPass/Velocity.h"

namespace ZE::GFX::Pipeline::RenderPass::Velocity
{
	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->State.Free(dev);
		delete execData;
	}

	ExecuteData* Setup(Device& dev, RendererBuildData& buildData, PixelFormat motionformat)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Current depth
		desc.AddRange({ 1, 1, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Prev depth
		desc.AddRange({ 1, 12, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::CBV | Binding::RangeFlag::GlobalBuffer }); // Renderer dynamic data
		desc.AppendSamplers(buildData.RendererSlots);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		const auto& schema = buildData.BindingLib.GetSchema(passData->BindingIndex);
		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, "VelocityPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Blender = Resource::BlendType::None;
		psoDesc.SetDepthClip(false);
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = motionformat;
		ZE_PSO_SET_NAME(psoDesc, "Velocity");
		passData->State.Init(dev, psoDesc, schema);

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Velocity");
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		cl.Open(dev, data.State);
		ZE_DRAW_TAG_BEGIN(dev, cl, "Velocity", PixelVal::White);
		ctx.BindingSchema.SetGraphics(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.CurrentDepth);
		renderData.Buffers.SetSRV(cl, ctx, ids.PrevDepth);
		renderData.BindRendererDynamicData(cl, ctx);
		renderData.Buffers.SetRTV(cl, ids.MotionVectors);
		cl.DrawFullscreen(dev);

		ZE_DRAW_TAG_END(dev, cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}