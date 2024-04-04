#include "GFX/Pipeline/RenderPass/HorizontalBlur.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::HorizontalBlur
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void*& initData)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for HorizontalBlur initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	PassDesc GetDesc(PixelFormat formatRT) noexcept
	{
		PassDesc desc{ static_cast<PassType>(CorePassType::HorizontalBlur) };
		desc.InitializeFormats.emplace_back(formatRT);
		desc.Init = Initialize;
		desc.Execute = Execute;
		desc.Clean = Clean;
		return desc;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ sizeof(U32), 0, 0, Resource::ShaderType::Pixel, Binding::RangeFlag::Constant }); // Direction
		desc.AddRange({ 1, 0, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // Blur texture
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::PipelineStateDesc psoDesc;
		psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
		psoDesc.SetShader(dev, psoDesc.PS, "BlurPS", buildData.ShaderCache);
		psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		ZE_PSO_SET_NAME(psoDesc, "HorizontalBlur");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Horizontal Blur");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		ZE_DRAW_TAG_BEGIN(dev, cl, "Outline Horizontal Blur", Pixel(0xF3, 0xEA, 0xAF));
		ctx.BindingSchema.SetGraphics(cl);
		// TODO: What about first usage of render targets when they get fully overwritten?
		// Need to handle such cases with barriers and discard operation
		//renderData.Buffers.InitRTV(cl, ids.RenderTarget);

		Resource::Constant<U32> direction(dev, false);
		direction.Bind(cl, ctx);
		//renderData.Buffers.SetSRV(cl, ctx, ids.Outline);
		renderData.SettingsBuffer.Bind(cl, ctx);
		//renderData.Buffers.SetRTV(cl, ids.RenderTarget);
		cl.DrawFullscreen(dev);

		ZE_DRAW_TAG_END(dev, cl);
	}
}