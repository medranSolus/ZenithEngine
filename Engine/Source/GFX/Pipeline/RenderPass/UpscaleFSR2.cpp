#include "GFX/Pipeline/RenderPass/UpscaleFSR2.h"
#include "GFX/FfxBackendInterface.h"
#include "Data/Camera.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleFSR2
{
	static bool Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void*& initData) { return Initialize(dev, buildData); }

	static void MessageHandler(FfxMsgType type, const wchar_t* msg) noexcept
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

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ static_cast<PassType>(CorePassType::UpscaleFSR2) };
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
		ZE_FFX_CHECK(ffxFsr2ContextDestroy(&execData->Ctx), "Error destroying FSR2 context!");
		delete execData;
	}

	bool Update(Device& dev, ExecuteData& passData, bool firstUpdate)
	{
		UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::Fsr2, passData.Quality);
		if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize)
		{
			ZE_FFX_ENABLE();
			Settings::RenderSize = renderSize;
			passData.DisplaySize = Settings::DisplaySize;

			if (!firstUpdate)
			{
				ZE_FFX_CHECK(ffxFsr2ContextDestroy(&passData.Ctx), "Error destroying FSR2 context!");
			}
			FfxFsr2ContextDescription ctxDesc = {};
			ctxDesc.flags = FFX_FSR2_ENABLE_DEPTH_INVERTED | FFX_FSR2_ENABLE_DEPTH_INFINITE
				| FFX_FSR2_ENABLE_AUTO_EXPOSURE | FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE
				| (_ZE_DEBUG_GFX_API ? FFX_FSR2_ENABLE_DEBUG_CHECKING : 0);
			ctxDesc.maxRenderSize = { renderSize.X, renderSize.Y };
			ctxDesc.displaySize = { passData.DisplaySize.X, passData.DisplaySize.Y };
			ctxDesc.backendInterface = dev.GetFfxInterface();
			ctxDesc.fpMessage = MessageHandler;
			ZE_FFX_THROW_FAILED(ffxFsr2ContextCreate(&passData.Ctx, &ctxDesc), "Error creating FSR2 context!");
			return true;
		}
		return false;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData)
	{
		ExecuteData* passData = new ExecuteData;
		Update(dev, *passData, true);
		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_FFX_ENABLE();
		ZE_PERF_GUARD("Upscale FSR2");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ExecuteData& data = *passData.ExecData.Cast<ExecuteData>();
		const UInt2 inputSize = renderData.Buffers.GetDimmensions(ids.Color);

		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale FSR2", Pixel(0xB2, 0x22, 0x22));

		FfxFsr2DispatchDescription desc = {};
		desc.commandList = ffxGetCommandList(cl);
		Resource::Generic color, depth, motion, reactiveMask, output;
		//desc.color = ffxGetResource(renderData.Buffers, color, ids.Color, Resource::StateShaderResourceNonPS);
		//desc.depth = ffxGetResource(renderData.Buffers, depth, ids.Depth, Resource::StateShaderResourceNonPS);
		//desc.motionVectors = ffxGetResource(renderData.Buffers, motion, ids.MotionVectors, Resource::StateShaderResourceNonPS);
		desc.exposure.resource = nullptr;
		//desc.reactive = ffxGetResource(renderData.Buffers, reactiveMask, ids.ReactiveMask, Resource::StateShaderResourceNonPS);
		desc.transparencyAndComposition.resource = nullptr; // Alpha value for special surfaces (reflections, animated textures, etc.), add when needed
		//desc.output = ffxGetResource(renderData.Buffers, output, ids.Output, Resource::StateUnorderedAccess);
		desc.jitterOffset.x = Data::GetUnitPixelJitterX(renderData.DynamicData.JitterCurrent.x, Settings::RenderSize.X);
		desc.jitterOffset.y = Data::GetUnitPixelJitterY(renderData.DynamicData.JitterCurrent.y, Settings::RenderSize.Y);
		desc.motionVectorScale.x = -Utils::SafeCast<float>(inputSize.X);
		desc.motionVectorScale.y = -Utils::SafeCast<float>(inputSize.Y);
		desc.renderSize = { inputSize.X, inputSize.Y };
		desc.enableSharpening = data.SharpeningEnabled;
		desc.frameTimeDelta = Utils::SafeCast<float>(Settings::FrameTime);
		desc.sharpness = data.Sharpness;
		desc.preExposure = 1.0f;
		desc.reset = false; // TODO: check conditions for that
		desc.cameraNear = FLT_MAX;
		desc.cameraFar = renderData.DynamicData.NearClip;
		desc.cameraFovAngleVertical = Settings::Data.get<Data::Camera>(renderData.GraphData.CurrentCamera).Projection.FOV;
		desc.viewSpaceToMetersFactor = 1.0f;
		desc.enableAutoReactive = false;
		desc.colorOpaqueOnly.resource = nullptr;
		desc.autoTcThreshold = 0.05f; // Setting this value too small will cause visual instability. Larger values can cause ghosting
		desc.autoTcScale = 1.0f; // Smaller values will increase stability at hard edges of translucent objects
		desc.autoReactiveScale = 5.00f; // Larger values result in more reactive pixels
		desc.autoReactiveMax = 0.90f; // Maximum value reactivity can reach
		ZE_FFX_THROW_FAILED(ffxFsr2ContextDispatch(&data.Ctx, &desc), "Error performing FSR2!");

		ZE_DRAW_TAG_END(dev, cl);
	}
}