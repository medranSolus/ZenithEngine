#include "GFX/Pipeline/RenderPass/LightCombine.h"

namespace ZE::GFX::Pipeline::RenderPass::LightCombine
{
	static bool Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for LightCombine initialization formats!");
		Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData), formats.front());
		return false;
	}

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for LightCombine initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	PassDesc GetDesc(PixelFormat outputFormat) noexcept
	{
		PassDesc desc{ Base(CorePassType::LightCombine) };
		desc.InitializeFormats.emplace_back(outputFormat);
		desc.Init = Initialize;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		return desc;
	}

	void Clean(Device& dev, void* data) noexcept
	{
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->State.Free(dev);
		delete execData;
	}

	void Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat)
	{
		const bool isAO = Settings::GetAOType() != AOType::None;
		if (isAO != passData.AmbientOcclusionEnabled)
		{
			passData.AmbientOcclusionEnabled = isAO;
			Resource::PipelineStateDesc psoDesc;
			psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
			psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
			psoDesc.Culling = Resource::CullMode::None;
			psoDesc.RenderTargetsCount = 1;
			psoDesc.FormatsRT[0] = outputFormat;
			psoDesc.SetShader(dev, psoDesc.PS, isAO ? "LightCombinePS_A" : "LightCombinePS", buildData.ShaderCache);
			ZE_PSO_SET_NAME(psoDesc, psoDesc.PS->GetName());
			passData.State.Free(dev);
			passData.State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData.BindingIndex));
		}
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 2, 0, 1, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack | Binding::RangeFlag::RangeSourceDynamic }); // Direct lighting + SSAO
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		passData->AmbientOcclusionEnabled = Settings::GetAOType() == AOType::None;
		Update(dev, buildData, *passData, outputFormat);

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Light Combine");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_ASSERT(data.AmbientOcclusionEnabled == (Settings::GetAOType() != AOType::None),
			"LightCombine pass not updated for changed ssao input settings!");

		ZE_DRAW_TAG_BEGIN(dev, cl, "LightCombine", Pixel(0xFF, 0xFF, 0x9F));
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.DirectLight);
		renderData.SettingsBuffer.Bind(cl, ctx);
		cl.DrawFullscreen(dev);

		renderData.Buffers.EndRaster(cl);
		ZE_DRAW_TAG_END(dev, cl);
	}
}