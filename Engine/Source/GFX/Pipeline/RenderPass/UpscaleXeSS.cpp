#include "GFX/Pipeline/RenderPass/UpscaleXeSS.h"
#include "GFX/XeSSException.h"
#include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleXeSS
{
	static UpdateStatus Update(Device& dev, RendererPassBuildData& buildData, void* passData, const std::vector<PixelFormat>& formats) { return Update(dev, *reinterpret_cast<ExecuteData*>(passData), buildData.SyncStatus); }

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
		PassDesc desc{ Base(CorePassType::UpscaleXeSS) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.Clean = Clean;
		desc.DebugUI = DebugUI;
		return desc;
	}

	void Clean(Device& dev, void* data, GpuSyncStatus& syncStatus)
	{
		Settings::RenderSize = Settings::DisplaySize;
		syncStatus.SyncMain(dev);
		dev.FreeXeSS();
		delete reinterpret_cast<ExecuteData*>(data);
	}

	UpdateStatus Update(Device& dev, ExecuteData& passData, GpuSyncStatus& syncStatus)
	{
		UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::XeSS, passData.Quality);
		if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize)
		{
			ZE_XESS_ENABLE();
			Settings::RenderSize = renderSize;
			passData.DisplaySize = Settings::DisplaySize;

			syncStatus.SyncMain(dev);
			if (dev.IsXeSSEnabled())
				dev.FreeXeSS();

			ZE_XESS_CHECK(xessSetLoggingCallback(dev.GetXeSSCtx(),
				_ZE_DEBUG_GFX_API ? XESS_LOGGING_LEVEL_DEBUG : XESS_LOGGING_LEVEL_WARNING, MessageHandler),
				"Error setting XeSS message callback!");
			ZE_XESS_CHECK(xessSetJitterScale(dev.GetXeSSCtx(), 1.0f, 1.0f),
				"Error setting XeSS jitter scale!");
			ZE_XESS_CHECK(xessSetVelocityScale(dev.GetXeSSCtx(),
				-Utils::SafeCast<float>(renderSize.X), -Utils::SafeCast<float>(renderSize.Y)),
				"Error setting XeSS motion vectors scale!");

			dev.InitializeXeSS(Settings::DisplaySize, passData.Quality,
				XESS_INIT_FLAG_INVERTED_DEPTH | XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE | XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK);
			return UpdateStatus::FrameBufferImpact;
		}
		return UpdateStatus::NoUpdate;
	}

	void* Initialize(Device& dev, RendererPassBuildData& buildData)
	{
		ExecuteData* passData = new ExecuteData;
		Update(dev, *passData, buildData.SyncStatus);
		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData)
	{
		ZE_PERF_GUARD("Upscale XeSS");

		Resources ids = *passData.Resources.CastConst<Resources>();
		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale XeSS", Pixel(0xB2, 0x22, 0x22));

		renderData.Buffers.ExecuteXeSS(dev, cl, ids.Color, ids.MotionVectors, ids.Depth, INVALID_RID, ids.ResponsiveMask, ids.Output,
			renderData.DynamicData.JitterCurrent.x, renderData.DynamicData.JitterCurrent.y, renderData.GraphData.FrameTemporalReset);
		cl.RestoreExternalState(dev);

		ZE_DRAW_TAG_END(dev, cl);
	}

	void DebugUI(void* data) noexcept
	{
		if (ImGui::CollapsingHeader("XeSS"))
		{
			ExecuteData& execData = *reinterpret_cast<ExecuteData*>(data);

			xess_version_t versionInfo = {};
			if (xessGetVersion(&versionInfo) == XESS_RESULT_SUCCESS)
				ImGui::Text("Version %" PRIu16 ".%" PRIu16 ".%" PRIu16, versionInfo.major, versionInfo.minor, versionInfo.patch);
			else
				ImGui::Text("Version 2.0.1 (built with)");

			constexpr std::array<const char*, 7> LEVELS = { "Ultra Performance", "Performance", "Balanced", "Quality", "Ultra Quality", "Ultra Quality Plus", "Native AA" };
			if (ImGui::BeginCombo("Quality level", LEVELS.at(static_cast<U8>(execData.Quality) - 100U)))
			{
				for (xess_quality_settings_t i = XESS_QUALITY_SETTING_ULTRA_PERFORMANCE; const char* level : LEVELS)
				{
					const bool selected = i == execData.Quality;
					if (ImGui::Selectable(level, selected))
						execData.Quality = i;
					if (selected)
						ImGui::SetItemDefaultFocus();
					i = static_cast<xess_quality_settings_t>(static_cast<U8>(i) + 1U);
				}
				ImGui::EndCombo();
			}
			ImGui::NewLine();
		}
	}
}