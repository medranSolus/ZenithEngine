#include "GFX/Pipeline/RenderPass/UpscaleXeSS.h"
#include "GFX/XeSSException.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleXeSS
{
	static bool Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, *reinterpret_cast<ExecuteData*>(passData)); }

	static void* Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) { return Initialize(dev, buildData); }

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

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ static_cast<PassType>(CorePassType::UpscaleXeSS) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		return desc;
	}

	bool Update(Device& dev, ExecuteData& passData)
	{
		UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::XeSS, passData.Quality);
		if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize)
		{
			ZE_XESS_ENABLE();
			Settings::RenderSize = renderSize;
			passData.DisplaySize = Settings::DisplaySize;

			ZE_XESS_CHECK(xessSetVelocityScale(dev.GetXeSSCtx(),
				-Utils::SafeCast<float>(renderSize.X), -Utils::SafeCast<float>(renderSize.Y)),
				"Error setting XeSS motion vectors scale!");

			dev.InitializeXeSS(Settings::DisplaySize, passData.Quality,
				XESS_INIT_FLAG_INVERTED_DEPTH | XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE | XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK);
			return true;
		}
		return false;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData)
	{
		ZE_XESS_ENABLE();
		ExecuteData* passData = new ExecuteData;

		ZE_XESS_CHECK(xessSetLoggingCallback(dev.GetXeSSCtx(),
			_ZE_DEBUG_GFX_API ? XESS_LOGGING_LEVEL_DEBUG : XESS_LOGGING_LEVEL_WARNING, MessageHandler),
			"Error setting XeSS message callback!");
		ZE_XESS_CHECK(xessSetJitterScale(dev.GetXeSSCtx(), 1.0f, 1.0f),
			"Error setting XeSS jitter scale!");
		Update(dev, *passData);

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Upscale XeSS");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale XeSS", Pixel(0xB2, 0x22, 0x22));

		renderData.Buffers.ExecuteXeSS(dev, cl, ids.Color, ids.MotionVectors, ids.Depth, INVALID_RID, ids.ResponsiveMask, ids.Output,
			renderData.DynamicData.JitterCurrent.x, renderData.DynamicData.JitterCurrent.y, false);

		ZE_DRAW_TAG_END(dev, cl);
	}
}