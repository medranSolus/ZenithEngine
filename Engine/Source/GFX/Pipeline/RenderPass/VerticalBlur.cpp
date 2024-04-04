#include "GFX/Pipeline/RenderPass/VerticalBlur.h"
#include "GFX/Resource/Constant.h"

namespace ZE::GFX::Pipeline::RenderPass::VerticalBlur
{
	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void*& initData)
	{
		ZE_ASSERT(formats.size() == 2, "Incorrect size for VerticalBlur initialization formats!");
		return Initialize(dev, buildData, formats.at(0), formats.at(1));
	}

	PassDesc GetDesc(PixelFormat formatRT, PixelFormat formatDS) noexcept
	{
		PassDesc desc{ static_cast<PassType>(CorePassType::VerticalBlur) };
		desc.InitializeFormats.reserve(2);
		desc.InitializeFormats.emplace_back(formatRT);
		desc.InitializeFormats.emplace_back(formatDS);
		desc.Init = Initialize;
		desc.Execute = Execute;
		desc.Clean = Clean;
		return desc;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatRT, PixelFormat formatDS)
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
		psoDesc.DepthStencil = Resource::DepthStencilMode::StencilMask;
		psoDesc.Culling = Resource::CullMode::None;
		psoDesc.Blender = Resource::BlendType::Normal;
		psoDesc.RenderTargetsCount = 1;
		psoDesc.FormatsRT[0] = formatRT;
		psoDesc.FormatDS = formatDS;
		ZE_PSO_SET_NAME(psoDesc, "VerticalBlur");
		passData->State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData->BindingIndex));

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Vertical Blur");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };

		ZE_DRAW_TAG_BEGIN(dev, cl, "Outline Vertical Blur", Pixel(0xFD, 0xEF, 0xB2));
		ctx.BindingSchema.SetGraphics(cl);

		data.State.SetStencilRef(cl, 0xFF);
		Resource::Constant<U32> direction(dev, true);
		direction.Bind(cl, ctx);
		//renderData.Buffers.SetSRV(cl, ctx, ids.OutlineBlur);
		renderData.SettingsBuffer.Bind(cl, ctx);
		//renderData.Buffers.SetOutput(cl, ids.RenderTarget, ids.DepthStencil);
		cl.DrawFullscreen(dev);

		ZE_DRAW_TAG_END(dev, cl);
	}
}