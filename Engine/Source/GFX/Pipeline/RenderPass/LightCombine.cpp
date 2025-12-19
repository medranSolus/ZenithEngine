#include "GFX/Pipeline/RenderPass/LightCombine.h"

namespace ZE::GFX::Pipeline::RenderPass::LightCombine
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for LightCombine initialization formats!");
		return Update(dev, buildData, *reinterpret_cast<ExecuteData*>(passData), formats.front());
	}

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for LightCombine initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	static std::string GetPsoName(bool isAO, bool isIBL, bool isSSR) noexcept
	{
		std::string base = "LightCombinePS";
		if (isAO || isIBL || isSSR)
			base += '_';
		if (isAO)
			base += 'A';
		if (isIBL)
			base += 'I';
		if (isSSR)
			base += 'R';
		return base;
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

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		syncStatus.SyncMain(dev);
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		execData->State.Free(dev);
		delete execData;
	}

	UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, ExecuteData& passData, PixelFormat outputFormat)
	{
		const bool isAO = Settings::AmbientOcclusionType != AOType::None;
		if (isAO != passData.AmbientOcclusionEnabled || Settings::IsEnabledIBL() != passData.IBLState || Settings::IsEnabledSSSR() != passData.SSRState)
		{
			passData.AmbientOcclusionEnabled = isAO;
			passData.IBLState = Settings::IsEnabledIBL();
			passData.SSRState = Settings::IsEnabledSSSR();

			Resource::PipelineStateDesc psoDesc;
			psoDesc.SetShader(dev, psoDesc.VS, "FullscreenVS", buildData.ShaderCache);
			psoDesc.DepthStencil = Resource::DepthStencilMode::DepthOff;
			psoDesc.Culling = Resource::CullMode::None;
			psoDesc.RenderTargetsCount = 1;
			psoDesc.FormatsRT[0] = outputFormat;
			psoDesc.SetShader(dev, psoDesc.PS, GetPsoName(passData.AmbientOcclusionEnabled, passData.IBLState, passData.SSRState), buildData.ShaderCache);
			ZE_PSO_SET_NAME(psoDesc, psoDesc.PS->GetName());

			buildData.SyncStatus.SyncMain(dev);
			passData.State.Free(dev);
			passData.State.Init(dev, psoDesc, buildData.BindingLib.GetSchema(passData.BindingIndex));
			return UpdateStatus::GraphImpact;
		}
		return UpdateStatus::NoUpdate;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat outputFormat)
	{
		ExecuteData* passData = new ExecuteData;

		Binding::SchemaDesc desc;
		desc.AddRange({ 3, 0, 2, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack | Binding::RangeFlag::RangeSourceDynamic }); // Direct lighting + SSAO + SSR
		desc.AddRange(buildData.DynamicDataRange, Resource::ShaderType::Pixel);
		desc.AddRange(buildData.SettingsRange, Resource::ShaderType::Pixel);
		desc.AddRange({ 4, 3, 3, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // GBuff
		desc.AddRange({ 1, 7, 4, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // EnvMap
		desc.AddRange({ 1, 8, 5, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // BRDF LUT
		desc.AddRange({ 1, 9, 6, Resource::ShaderType::Pixel, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack }); // IrrMap
		desc.AppendSamplers(buildData.Samplers);
		passData->BindingIndex = buildData.BindingLib.AddDataBinding(dev, desc);

		passData->AmbientOcclusionEnabled = Settings::AmbientOcclusionType == AOType::None;
		passData->IBLState = !Settings::IsEnabledIBL();
		Update(dev, buildData, *passData, outputFormat);

		return passData;
	}

	bool Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Light Combine");
		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();

		ZE_ASSERT(data.AmbientOcclusionEnabled == (Settings::AmbientOcclusionType != AOType::None),
			"LightCombine pass not updated for changed ssao input settings!");
		ZE_ASSERT(data.IBLState == Settings::IsEnabledIBL(),
			"LightCombine pass not updated for changed IBL settings!");
		ZE_ASSERT(data.SSRState == Settings::IsEnabledSSSR(),
			"LightCombine pass not updated for changed SSSR settings!");

		ZE_DRAW_TAG_BEGIN(dev, cl, "LightCombine", Pixel(0xFF, 0xFF, 0x9F));
		renderData.Buffers.BeginRaster(cl, ids.RenderTarget);

		Binding::Context ctx{ renderData.Bindings.GetSchema(data.BindingIndex) };
		ctx.BindingSchema.SetGraphics(cl);
		data.State.Bind(cl);

		renderData.Buffers.SetSRV(cl, ctx, ids.DirectLight);
		renderData.BindRendererDynamicData(cl, ctx);
		renderData.SettingsBuffer.Bind(cl, ctx);
		if (data.IBLState || data.SSRState)
		{
			renderData.Buffers.SetSRV(cl, ctx, ids.GBufferDepth);
			renderData.Buffers.SetSRV(cl, ctx, ids.EnvMap);
			renderData.Buffers.SetSRV(cl, ctx, ids.BrdfLUT);
			if (data.IBLState)
				renderData.Buffers.SetSRV(cl, ctx, ids.IrrMap);
		}
		cl.DrawFullscreen(dev);

		renderData.Buffers.EndRaster(cl);
		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}
}