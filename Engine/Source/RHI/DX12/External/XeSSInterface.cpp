#if _ZE_XESS_ENABLED
#	include "RHI/DX12/External/XeSSInterface.h"
#	include "GFX/External/Error.h"
#	include "Data/Camera.h"
ZE_WARNING_PUSH
#	include "xess/xess_d3d12.h"
ZE_WARNING_POP

namespace ZE::RHI::DX12::External
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
		if (descInfo.Handle)
			GarbageCollector::Get().Register(GarbageCollector::Get().MarkInactive(descInfo.Handle), std::move(descInfo));
	}

	void XeSSInterface::MoveFrom(XeSSInterface&& xess) noexcept
	{
		ctx = std::exchange(xess.ctx, nullptr);
		descInfo = std::move(xess.descInfo);
	}

	Status XeSSInterface::CreateCtx(Device& dev) noexcept
	{
		ZE_XESS_LOG_RET_FAILED(xessD3D12CreateContext(dev.GetDevice(), &ctx),
			"Error creating XeSS D3D12 context!");

		ZE_XESS_LOG_RET_FAILED(xessSetLoggingCallback(ctx,
			_ZE_DEBUG_GFX_API ? XESS_LOGGING_LEVEL_DEBUG : XESS_LOGGING_LEVEL_WARNING, MessageHandler),
			"Error setting XeSS message callback!");

		return {};
	}

	Expected<XeSSInterface> XeSSInterface::Create(GFX::Device& dev) noexcept
	{
		XeSSInterface xess;
		ZE_CODE_RET_FAILED_EXPECT(xess.CreateCtx(dev.Get().dx12));

		if (xessIsOptimalDriver(xess.ctx) == XESS_RESULT_WARNING_OLD_DRIVER)
			Logger::Warning("Outdated Intel driver!");

		return xess;
	}

	Expected<U64> XeSSInterface::GetAliasableBufferRegionSize(UInt2 targetRes) const noexcept
	{
		xess_properties_t props = {};
		xess_2d_t outputRes = { targetRes.X, targetRes.Y };
		ZE_XESS_LOG_RET_FAILED_EXPECT(xessGetProperties(ctx, &outputRes, &props), "Error querying XeSS properties!");
		return props.tempBufferHeapSize;
	}

	Expected<U64> XeSSInterface::GetAliasableTextureRegionSize(UInt2 targetRes) const noexcept
	{
		xess_properties_t props = {};
		xess_2d_t outputRes = { targetRes.X, targetRes.Y };
		ZE_XESS_LOG_RET_FAILED_EXPECT(xessGetProperties(ctx, &outputRes, &props), "Error querying XeSS properties!");
		return props.tempTextureHeapSize;
	}

	Status XeSSInterface::InitializeCtx(GFX::Device& dev, GFX::Pipeline::FrameBuffer& buffers, UInt2 targetRes,
		xess_quality_settings_t quality, U32 flags, RID aliasableBuffer, RID aliasableTexture) noexcept
	{
		ZE_ASSERT(!IsCtxInitialized(), "XeSS Ctx already initialized!");

		auto& framebuffer = buffers.Get().dx12;
		auto& device = dev.Get().dx12;

		xess_d3d12_init_params_t initParams = {};
		initParams.outputResolution = { targetRes.X, targetRes.Y };
		initParams.qualitySetting = quality;
		initParams.initFlags = flags | XESS_INIT_FLAG_EXTERNAL_DESCRIPTOR_HEAP;
		initParams.creationNodeMask = 0;
		initParams.visibleNodeMask = 0;
		initParams.pTempBufferHeap = framebuffer.GetHeapBuffer();
		initParams.bufferHeapOffset = framebuffer.GetHeapOffset(aliasableBuffer, device.IsTightAlignment());
		initParams.pTempTextureHeap = framebuffer.GetHeapUAV();
		initParams.textureHeapOffset = framebuffer.GetHeapOffset(aliasableTexture, device.IsTightAlignment());
		initParams.pPipelineLibrary = nullptr;

		ZE_XESS_LOG_RET_FAILED(xessD3D12BuildPipelines(ctx, nullptr, false, initParams.initFlags), "Error building XeSS D3D12 pipelines!");

		// Init external descriptor pool
		xess_properties_t props = {};
		ZE_XESS_LOG_RET_FAILED(xessGetProperties(ctx, &initParams.outputResolution, &props), "Error querying XeSS properties!");

		ZE_EXPECT_RET_FAILED_CODE(descInfo, device.AllocDescs(props.requiredDescriptorCount * Settings::GetBackbufferCount()));
		GarbageCollector::Get().MarkActive(device, descInfo.Handle);

		// Finish initialization
		ZE_XESS_LOG_RET_FAILED(xessD3D12Init(ctx, &initParams), "Failed to initialize XeSS D3D12 context!");

		return {};
	}

	Status XeSSInterface::FreeCtx(GFX::Device& dev) noexcept
	{
		ZE_ASSERT(IsCtxInitialized(), "XeSS Ctx not initialized!");

		Destroy();
		return CreateCtx(dev.Get().dx12);
	}

	Status XeSSInterface::Execute(GFX::Device& dev, GFX::Pipeline::FrameBuffer& buffers, GFX::CommandList& cl,
		RID color, RID motionVectors, RID depth, RID exposure, RID responsive, RID output, const Float2& jitter, bool reset) const noexcept
	{
		ZE_ASSERT(IsCtxInitialized(), "XeSS Ctx not initialized!");

		Device& device = dev.Get().dx12;
		Pipeline::FrameBuffer& frameBuff = buffers.Get().dx12;

		xess_d3d12_execute_params_t execParams = {};
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
		execParams.pDescriptorHeap = device.GetDescHeap();

		U32 descSize = device.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		U32 singleSetCount = device.GetAllocatedDescsCount(descInfo) / Settings::GetBackbufferCount();
		execParams.descriptorHeapOffset = Utils::SafeCast<U32>(descInfo.GPU.ptr - execParams.pDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr);
		execParams.descriptorHeapOffset += descSize * singleSetCount * Settings::GetCurrentBackbufferIndex();

		return ZE_XESS_ERROR(xessD3D12Execute(ctx, cl.Get().dx12.GetList(), &execParams));
	}
}
#endif