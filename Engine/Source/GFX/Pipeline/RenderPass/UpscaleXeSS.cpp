#include "GFX/Pipeline/RenderPass/UpscaleXeSS.h"
#include "GFX/Pipeline/RendererPBR.h"
#include "GFX/Resource/Generic.h"
#include "GFX/XeSSException.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleXeSS
{
	void MessageHandler(const char* message, xess_logging_level_t level) noexcept
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

	void* Setup(Device& dev)
	{
		ZE_XESS_ENABLE();

		dev.InitializeXeSS(Settings::DisplaySize, XESS_QUALITY_SETTING_ULTRA_QUALITY,
			XESS_INIT_FLAG_INVERTED_DEPTH | XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE | XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK);

		ZE_XESS_CHECK(xessSetLoggingCallback(dev.GetXeSSCtx(),
			_ZE_DEBUG_GFX_API ? XESS_LOGGING_LEVEL_DEBUG : XESS_LOGGING_LEVEL_WARNING, MessageHandler),
			"Error setting XeSS message callback!");
		ZE_XESS_CHECK(xessSetVelocityScale(dev.GetXeSSCtx(),
			-Utils::SafeCast<float>(Settings::RenderSize.X), -Utils::SafeCast<float>(Settings::RenderSize.Y)),
			"Error setting XeSS motion vectors scale!");
		ZE_XESS_CHECK(xessSetJitterScale(dev.GetXeSSCtx(), 1.0f, 1.0f),
			"Error setting XeSS jitter scale!");

		return nullptr;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Upscale XeSS");

		Resources ids = *passData.Buffers.CastConst<Resources>();
		const Data::Projection& projection = reinterpret_cast<RendererPBR*>(renderData.Renderer)->GetProjectionData();

		cl.Open(dev);
		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale XeSS", Pixel(0xB2, 0x22, 0x22));

		// Create proxy resources
		Resource::Generic color(renderData.Buffers, ids.Color),
			motionVectors(renderData.Buffers, ids.MotionVectors),
			depth(renderData.Buffers, ids.Depth),
			responsive(renderData.Buffers, ids.ResponsiveMask),
			output(renderData.Buffers, ids.Output);

		dev.ExecuteXeSS(cl, color, motionVectors, &depth, nullptr, &responsive, output, projection.JitterX, projection.JitterY, Settings::RenderSize, false);

		// Free proxy resources
		color.Free(dev);
		motionVectors.Free(dev);
		depth.Free(dev);
		responsive.Free(dev);
		output.Free(dev);

		ZE_DRAW_TAG_END(dev, cl);
		cl.Close(dev);
		dev.ExecuteMain(cl);
	}
}