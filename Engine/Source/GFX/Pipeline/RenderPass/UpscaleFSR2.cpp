#include "GFX/Pipeline/RenderPass/UpscaleFSR2.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/FfxBackendInterface.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR2
{
	void MessageHandler(FfxMsgType type, const wchar_t* msg) noexcept
	{
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FFX_MESSAGE_TYPE_ERROR:
			Logger::Error(Utils::ToUTF8(msg));
			break;
		case FFX_MESSAGE_TYPE_WARNING:
			Logger::Warning(Utils::ToUTF8(msg));
			break;
		}
	}

	void Clean(Device& dev, void* data) noexcept
	{
		ZE_FFX_ENABLE();
		ExecuteData* execData = reinterpret_cast<ExecuteData*>(data);
		ZE_FFX_CHECK(ffxFsr2ContextDestroy(&execData->Ctx), "Error destroying FSR2 context!");
		delete execData;
	}

	ExecuteData* Setup(Device& dev)
	{
		ZE_FFX_ENABLE();
		ExecuteData* passData = new ExecuteData;

		FfxFsr2ContextDescription ctxDesc = {};
		ctxDesc.flags = FFX_FSR2_ENABLE_DEPTH_INVERTED | FFX_FSR2_ENABLE_DEPTH_INFINITE
			| FFX_FSR2_ENABLE_AUTO_EXPOSURE | FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE
			| (_ZE_DEBUG_GFX_API ? FFX_FSR2_ENABLE_DEBUG_CHECKING : 0);
		ctxDesc.maxRenderSize = { Settings::RenderSize.X, Settings::RenderSize.Y };
		ctxDesc.displaySize = { Settings::DisplaySize.X, Settings::DisplaySize.Y };
		ctxDesc.backendInterface = dev.GetFfxInterface();
		ctxDesc.fpMessage = MessageHandler;
		ZE_FFX_THROW_FAILED(ffxFsr2ContextCreate(&passData->Ctx, &ctxDesc), "Error creating FSR2 context!");

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("Upscale FSR2");

		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);

		const RendererPBR& renderer = *reinterpret_cast<RendererPBR*>(renderData.Renderer);
		const Data::Projection& projection = renderer.GetProjectionData();

		cl.Open(dev);
		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale FSR2", Pixel(0xB2, 0x22, 0x22));

		FfxFsr2DispatchDescription desc = {};
		desc.commandList = ffxGetCommandList(cl);
		Resource::Generic color, depth, motion, reactiveMask, output;
		desc.color = ffxGetResource(renderData.Buffers, color, ids.Color, Resource::StateShaderResourceNonPS);
		desc.depth = ffxGetResource(renderData.Buffers, depth, ids.Depth, Resource::StateShaderResourceNonPS);
		desc.motionVectors = ffxGetResource(renderData.Buffers, motion, ids.MotionVectors, Resource::StateShaderResourceNonPS);
		desc.exposure.resource = nullptr;
		desc.reactive = ffxGetResource(renderData.Buffers, reactiveMask, ids.ReactiveMask, Resource::StateShaderResourceNonPS);
		desc.transparencyAndComposition.resource = nullptr; // Alpha value for special surfaces (reflections, animated textures, etc.), add when needed
		desc.output = ffxGetResource(renderData.Buffers, output, ids.Output, Resource::StateUnorderedAccess);
		desc.jitterOffset.x = Data::GetUnitPixelJitterX(projection.JitterX, Settings::RenderSize.X);
		desc.jitterOffset.y = Data::GetUnitPixelJitterY(projection.JitterY, Settings::RenderSize.Y);
		desc.motionVectorScale.x = -Utils::SafeCast<float>(inputSize.X);
		desc.motionVectorScale.y = -Utils::SafeCast<float>(inputSize.Y);
		desc.renderSize = { inputSize.X, inputSize.Y };
		desc.enableSharpening = renderer.IsSharpeningEnabled();
		desc.frameTimeDelta = Utils::SafeCast<float>(Settings::FrameTime);
		desc.sharpness = renderer.GetSharpness();
		desc.preExposure = 1.0f;
		desc.reset = false; // TODO: check conditions for that
		desc.cameraNear = FLT_MAX;
		desc.cameraFar = projection.NearClip;
		desc.cameraFovAngleVertical = projection.FOV;
		desc.viewSpaceToMetersFactor = 1.0f;
		desc.enableAutoReactive = false;
		desc.colorOpaqueOnly.resource = nullptr;
		desc.autoTcThreshold = 0.05f; // Setting this value too small will cause visual instability. Larger values can cause ghosting
		desc.autoTcScale = 1.0f; // Smaller values will increase stability at hard edges of translucent objects
		desc.autoReactiveScale = 5.00f; // Larger values result in more reactive pixels
		desc.autoReactiveMax = 0.90f; // Maximum value reactivity can reach
		ZE_FFX_THROW_FAILED(ffxFsr2ContextDispatch(&data.Ctx, &desc), "Error performing FSR2!");

		ZE_DRAW_TAG_END(dev, cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}