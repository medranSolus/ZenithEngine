#include "GFX/Pipeline/RenderPass/UpscaleFSR1.h"
#include "GFX/FfxBackendInterface.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR1
{
	static bool Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for FSR1 update formats!");
		return Update(dev, *reinterpret_cast<ExecuteData*>(passData), formats.front());
	}

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData)
	{
		ZE_ASSERT(formats.size() == 1, "Incorrect size for FSR1 initialization formats!");
		return Initialize(dev, buildData, formats.front());
	}

	PassDesc GetDesc(PixelFormat formatOutput) noexcept
	{
		PassDesc desc{ static_cast<PassType>(CorePassType::UpscaleFSR1) };
		desc.InitializeFormats.emplace_back(formatOutput);
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		return desc;
	}

	void Clean(Device& dev, void* data) noexcept
	{
		ZE_FFX_ENABLE();
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		ZE_FFX_CHECK(ffxFsr1ContextDestroy(&execData->Ctx), "Error destroying FSR1 context!");
		delete execData;
	}

	bool Update(Device& dev, ExecuteData& passData, PixelFormat formatOutput, bool firstUpdate)
	{
		UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::Fsr1, passData.Quality);
		if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize)
		{
			ZE_FFX_ENABLE();
			Settings::RenderSize = renderSize;
			passData.DisplaySize = Settings::DisplaySize;

			if (!firstUpdate)
			{
				ZE_FFX_CHECK(ffxFsr1ContextDestroy(&passData.Ctx), "Error destroying FSR1 context!");
			}
			FfxFsr1ContextDescription ctxDesc = {};
			ctxDesc.flags = FFX_FSR1_ENABLE_HIGH_DYNAMIC_RANGE | FFX_FSR1_ENABLE_RCAS;
			ctxDesc.outputFormat = GetFfxSurfaceFormat(formatOutput);
			ctxDesc.maxRenderSize = { renderSize.X, renderSize.Y };
			ctxDesc.displaySize = { passData.DisplaySize.X, passData.DisplaySize.Y };
			ctxDesc.backendInterface = dev.GetFfxInterface();
			ZE_FFX_THROW_FAILED(ffxFsr1ContextCreate(&passData.Ctx, &ctxDesc), "Error creating FSR1 context!");
			return true;
		}
		return false;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData, PixelFormat formatOutput)
	{
		ExecuteData* passData = new ExecuteData;
		Update(dev, *passData, formatOutput, true);
		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("Upscale FSR1");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);

		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale FSR1", Pixel(0xC2, 0x32, 0x32));

		FfxFsr1DispatchDescription desc = {};
		desc.commandList = ffxGetCommandList(cl);
		//Resource::Generic color, output;
		//desc.color = ffxGetResource(renderData.Buffers, color, ids.Color, Resource::StateShaderResourceNonPS);
		//desc.output = ffxGetResource(renderData.Buffers, output, ids.Output, Resource::StateUnorderedAccess);
		desc.renderSize = { inputSize.X, inputSize.Y };
		desc.enableSharpening = data.SharpeningEnabled;
		desc.sharpness = data.Sharpness;
		ZE_FFX_THROW_FAILED(ffxFsr1ContextDispatch(&data.Ctx, &desc), "Error performing FSR1!");

		ZE_DRAW_TAG_END(dev, cl);
	}
}