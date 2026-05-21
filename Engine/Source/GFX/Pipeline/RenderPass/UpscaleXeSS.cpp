#if _ZE_XESS_ENABLED
#	include "GFX/Pipeline/RenderPass/UpscaleXeSS.h"
#	include "GFX/External/InterfaceStorage.h"
#	include "GUI/DearImGui.h"

namespace ZE::GFX::Pipeline::RenderPass::UpscaleXeSS
{
	static void FlushGPU(Device* dev) noexcept
	{
		static Device* srcDev = nullptr;
		if (dev)
			srcDev = dev;
		else
		{
			ZE_ASSERT(srcDev, "No source device to flush GPU before releasing XeSS context!");
			Status code = srcDev->FlushGPU();
			if (code)
			{
				ZE_CODE_ERROR(code, "Failed to flush GPU for XeSS context destruction, race condition may happen!");
			}
		}
	}

	ExecuteData::~ExecuteData()
	{
		if (External::InterfaceStorage::GetConnectionXeSS())
		{
			Settings::RenderSize = Settings::DisplaySize;
			FlushGPU(nullptr);
			External::InterfaceStorage::ReleaseConnectionXeSS();
		}
	}

	static Expected<UpdateOperation> Update(Device& dev, RendererPassBuildData& buildData, PassExecuteData* passData, const std::vector<PixelFormat>& formats) noexcept
	{
		return Update(dev, *static_cast<ExecuteData*>(passData));
	}

	static ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData, const std::vector<PixelFormat>& formats, void* initData) noexcept
	{
		return Initialize(dev, buildData);
	}

	PassDesc GetDesc() noexcept
	{
		PassDesc desc{ Base(CorePassType::UpscaleXeSS) };
		desc.Init = Initialize;
		desc.Evaluate = Evaluate;
		desc.Execute = Execute;
		desc.Update = Update;
		desc.DebugUI = DebugUI;
		return desc;
	}

	Expected<UpdateOperation> Update(Device& dev, ExecuteData& passData) noexcept
	{
		auto xess = External::InterfaceStorage::GetConnectionXeSS();
		if (!xess)
			return std::unexpected(ZE_XESS_ERROR(XESS_RESULT_ERROR_UNINITIALIZED));

		UInt2 renderSize = CalculateRenderSize(dev, Settings::DisplaySize, UpscalerType::XeSS, passData.Quality);
		if (renderSize != Settings::RenderSize || passData.DisplaySize != Settings::DisplaySize)
		{
			if (xess->IsCtxInitialized())
			{
				FlushGPU(nullptr);
				ZE_CODE_RET_FAILED_EXPECT(xess->FreeCtx(dev));
			}
			passData.DisplaySize = Settings::DisplaySize;

			xess_context_handle_t ctx = xess->GetCtx();
			ZE_XESS_LOG_RET_FAILED_EXPECT(xessSetJitterScale(ctx, 1.0f, 1.0f),
				"Error setting XeSS jitter scale!");
			ZE_XESS_LOG_RET_FAILED_EXPECT(xessSetVelocityScale(ctx,
				-Utils::SafeCast<float>(renderSize.X), -Utils::SafeCast<float>(renderSize.Y)),
				"Error setting XeSS motion vectors scale!");

			ZE_CODE_RET_FAILED_EXPECT(xess->InitializeCtx(dev, passData.DisplaySize, passData.Quality,
				XESS_INIT_FLAG_INVERTED_DEPTH | XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE | XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK));

			Settings::RenderSize = renderSize;
			return UpdateOperation::FrameBufferImpact;
		}
		return UpdateOperation::NoUpdate;
	}

	ExpectedPassExecuteData Initialize(Device& dev, RendererPassBuildData& buildData) noexcept
	{
		auto xess = External::InterfaceStorage::CreateConnectionXeSS(dev);
		if (!xess)
			return std::unexpected(ZE_XESS_ERROR(XESS_RESULT_ERROR_UNKNOWN));

		auto passData = std::make_shared<ExecuteData>();
		auto operation = Update(dev, *passData);
		if (!operation)
			return std::unexpected(operation.error());
		FlushGPU(&dev);
		// Force update for framebuffer even if not signaled from render graph
		// since due to internals requiring memory from framebuffer
		buildData.FrameBufferUpdatePending |= xess->IsAliasableResourcesSupported();
		return passData;
	}

	Expected<bool> Execute(Device& dev, CommandList& cl, RendererPassExecuteData& renderData, PassData& passData) noexcept
	{
		auto xess = External::InterfaceStorage::GetConnectionXeSS();
		if (!xess)
			return std::unexpected(ZE_XESS_ERROR(XESS_RESULT_ERROR_UNINITIALIZED));

		ZE_PERF_GUARD("Upscale XeSS");

		Resources ids = *reinterpret_cast<Resources*>(passData.Resources.get());
		ZE_DRAW_TAG_BEGIN(dev, cl, "Upscale XeSS", Pixel(0xB2, 0x22, 0x22));

		ZE_CODE_RET_FAILED_EXPECT(xess->Execute(dev, renderData.Buffers, cl,
			ids.Color, ids.MotionVectors, ids.Depth, INVALID_RID, ids.ResponsiveMask, ids.Output,
			renderData.DynamicData.JitterCurrent, renderData.GraphData.FrameTemporalReset));
		cl.RestoreExternalState(dev);

		ZE_DRAW_TAG_END(dev, cl);
		return true;
	}

	void DebugUI(PassExecuteData* data) noexcept
	{
		if (ImGui::CollapsingHeader("XeSS"))
		{
			ExecuteData& execData = *static_cast<ExecuteData*>(data);

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
#endif