#include "GFX/Pipeline/RenderPass/UpscaleFSR1.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/FfxBackendInterface.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR1
{
	void Clean(Device& dev, void* data) noexcept
	{
		ZE_FFX_ENABLE();
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		ZE_FFX_CHECK(ffxFsr1ContextDestroy(&execData->Ctx), "Error destroying FSR2 context!");
		delete execData;
	}

	ExecuteData* Setup(Device& dev, PixelFormat formatOutput)
	{
		ZE_FFX_ENABLE();
		ExecuteData* passData = new ExecuteData;

		FfxFsr1ContextDescription ctxDesc = {};
		ctxDesc.flags = FFX_FSR1_ENABLE_HIGH_DYNAMIC_RANGE | FFX_FSR1_ENABLE_RCAS;
		ctxDesc.outputFormat = GetFfxSurfaceFormat(formatOutput);
		ctxDesc.maxRenderSize = { Settings::RenderSize.X, Settings::RenderSize.Y };
		ctxDesc.displaySize = { Settings::DisplaySize.X, Settings::DisplaySize.Y };
		ctxDesc.backendInterface = dev.GetFfxInterface();
		ZE_FFX_THROW_FAILED(ffxFsr1ContextCreate(&passData->Ctx, &ctxDesc), "Error creating FSR1 context!");

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("Upscale FSR1");

		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);

		const RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);

		cl.Open(dev);
		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale FSR1", Pixel(0xC2, 0x32, 0x32));

		FfxFsr1DispatchDescription desc = {};
		desc.commandList = ffxGetCommandList(cl);
		Resource::Generic color, output;
		desc.color = ffxGetResource(renderData.Buffers, color, ids.Color, Resource::StateShaderResourceNonPS);
		desc.output = ffxGetResource(renderData.Buffers, output, ids.Output, Resource::StateUnorderedAccess);
		desc.renderSize = { inputSize.X, inputSize.Y };
		desc.enableSharpening = renderer.IsSharpeningEnabled();
		desc.sharpness = renderer.GetSharpness();
		ZE_FFX_THROW_FAILED(ffxFsr1ContextDispatch(&data.Ctx, &desc), "Error performing FSR1!");

		ZE_DRAW_TAG_END(dev, cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}