#if _ZE_XESS_ENABLED
#	include "RHI/DX11/External/XeSSInterface.h"
#	include "GFX/External/Error.h"
#	include "Data/Camera.h"
ZE_WARNING_PUSH
#	include "xess/xess_d3d11.h"
ZE_WARNING_POP

namespace ZE::RHI::DX11::External
{
	static void MessageHandler(const char* message, xess_logging_level_t level) noexcept
	{
		switch (level)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case XESS_LOGGING_LEVEL_ERROR:
			Logger::Error(message);
			break;
		case XESS_LOGGING_LEVEL_WARNING:
			Logger::Warning(message);
			break;
		case XESS_LOGGING_LEVEL_INFO:
		case XESS_LOGGING_LEVEL_DEBUG:
			Logger::Info(message);
			break;
		}
	}

	void XeSSInterface::Destroy() noexcept
	{
		if (ctx)
		{
			ZE_XESS_CHECK(xessDestroyContext(ctx), "Error destroying XeSS context!");
			ctx = nullptr;
		}
		ctxInit = false;
	}

	void XeSSInterface::MoveFrom(XeSSInterface&& xess) noexcept
	{
		ctx = std::exchange(xess.ctx, nullptr);
		ctxInit = xess.ctxInit;
	}

	Status XeSSInterface::CreateCtx(Device& dev) noexcept
	{
		ZE_XESS_LOG_RET_FAILED(xessD3D11CreateContext(dev.GetDevice(), &ctx),
			"Error creating XeSS D3D11 context!");

		ZE_XESS_LOG_RET_FAILED(xessSetLoggingCallback(ctx,
			_ZE_DEBUG_GFX_API ? XESS_LOGGING_LEVEL_DEBUG : XESS_LOGGING_LEVEL_WARNING, MessageHandler),
			"Error setting XeSS message callback!");

		return {};
	}

	Expected<XeSSInterface> XeSSInterface::Create(GFX::Device& dev) noexcept
	{
		XeSSInterface xess;
		ZE_CODE_RET_FAILED_EXPECT(xess.CreateCtx(dev.Get().dx11));

		if (xessIsOptimalDriver(xess.ctx) == XESS_RESULT_WARNING_OLD_DRIVER)
			Logger::Warning("Outdated Intel driver!");

		return xess;
	}

	Status XeSSInterface::InitializeCtx(GFX::Device& dev, UInt2 targetRes, xess_quality_settings_t quality, U32 flags) noexcept
	{
		ZE_ASSERT(!IsCtxInitialized(), "XeSS Ctx already initialized!");

		xess_d3d11_init_params_t initParams = {};
		initParams.outputResolution = { targetRes.X, targetRes.Y };
		initParams.qualitySetting = quality;
		initParams.initFlags = flags;
		ZE_XESS_LOG_RET_FAILED(xessD3D11Init(ctx, &initParams), "Failed to initialize XeSS D3D11 context!");
		ctxInit = true;

		return {};
	}

	Status XeSSInterface::FreeCtx(GFX::Device& dev) noexcept
	{
		ZE_ASSERT(IsCtxInitialized(), "XeSS Ctx not initialized!");

		Destroy();
		return CreateCtx(dev.Get().dx11);
	}

	Status XeSSInterface::Execute(GFX::Device& dev, GFX::Pipeline::FrameBuffer& buffers, GFX::CommandList& cl,
		RID color, RID motionVectors, RID depth, RID exposure, RID responsive, RID output, const Float2& jitter, bool reset) const noexcept
	{
		ZE_ASSERT(IsCtxInitialized(), "XeSS Ctx not initialized!");

		Pipeline::FrameBuffer& frameBuff = buffers.Get().dx11;

		xess_d3d11_execute_params_t execParams = {};
		execParams.pColorTexture = frameBuff.GetResource(color).Get();
		execParams.pVelocityTexture = frameBuff.GetResource(motionVectors).Get();
		execParams.pDepthTexture = depth != INVALID_RID ? frameBuff.GetResource(depth).Get() : nullptr;
		execParams.pExposureScaleTexture = exposure != INVALID_RID ? frameBuff.GetResource(exposure).Get() : nullptr;
		execParams.pResponsivePixelMaskTexture = responsive != INVALID_RID ? frameBuff.GetResource(responsive).Get() : nullptr;
		execParams.pOutputTexture = frameBuff.GetResource(output).Get();

		UInt2 renderSize = frameBuff.GetDimmensions(color);
		execParams.jitterOffsetX = Data::GetUnitPixelJitterX(jitter.x, renderSize.X);
		execParams.jitterOffsetY = Data::GetUnitPixelJitterY(jitter.y, renderSize.Y);
		execParams.exposureScale = 1.0f;
		execParams.resetHistory = static_cast<U32>(reset);
		execParams.inputWidth = renderSize.X;
		execParams.inputHeight = renderSize.Y;
		execParams.inputColorBase = { 0, 0 };
		execParams.inputMotionVectorBase = { 0, 0 };
		execParams.inputDepthBase = { 0, 0 };
		execParams.inputResponsiveMaskBase = { 0, 0 };
		execParams.outputColorBase = { 0, 0 };

		return ZE_XESS_ERROR(xessD3D11Execute(ctx, &execParams));
	}
}
#endif