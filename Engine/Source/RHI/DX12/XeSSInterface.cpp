#if _ZE_XESS_ENABLED
#	include "RHI/DX12/XeSSInterface.h"
#	include "GFX/Error.h"
#	include "Data/Camera.h"
ZE_WARNING_PUSH
#	include "xess/xess_d3d12.h"
ZE_WARNING_POP

namespace ZE::RHI::DX12
{
	void XeSSInterface::Destroy(bool destroyCtx) noexcept
	{
		if (ctx && destroyCtx)
		{
			ZE_XESS_CHECK(xessDestroyContext(ctx), "Error destroying XeSS context!");
		}
		if (descInfo.Handle)
		{
			ZE_ASSERT(srcDev, "No source Device for cleanup!");
			srcDev->FreeDescs(descInfo);
		}

		outputRes = {};
		qualityMode = XESS_QUALITY_SETTING_AA;
		initFlags = 0;
		aliasBufferRegionSize = 0;
		aliasTextureRegionSize = 0;
		aliasBufferRegion = INVALID_RID;
		aliasTextureRegion = INVALID_RID;
	}

	void XeSSInterface::MoveFrom(XeSSInterface&& xess) noexcept
	{
		ctx = std::exchange(xess.ctx, nullptr);
		descInfo = std::move(xess.descInfo);
		outputRes = xess.outputRes;
		qualityMode = xess.qualityMode;
		initFlags = xess.initFlags;
		aliasBufferRegionSize = xess.aliasBufferRegionSize;
		aliasTextureRegionSize = xess.aliasTextureRegionSize;
		aliasBufferRegion = xess.aliasBufferRegion;
		aliasTextureRegion = xess.aliasTextureRegion;
		srcDev = xess.srcDev;
	}

	Expected<XeSSInterface> XeSSInterface::Create(GFX::Device& dev) noexcept
	{
		XeSSInterface xess;

		ZE_XESS_LOG_RET_FAILED_EXPECT(xessD3D12CreateContext(dev.Get().dx12.GetDevice(), &xess.ctx), "Error creating XeSS D3D12 context!");
		
		if (xessIsOptimalDriver(xess.ctx) == XESS_RESULT_WARNING_OLD_DRIVER)
			Logger::Warning("Outdated Intel driver!");

		return xess;
	}

	Status XeSSInterface::InitializeCtx(GFX::Device& dev, UInt2 targetRes, xess_quality_settings_t quality, U32 flags) noexcept
	{
		ZE_ASSERT(!IsInitialized(), "XeSS already initialized!");

		outputRes = { targetRes.X, targetRes.Y };
		qualityMode = quality;
		initFlags = flags | XESS_INIT_FLAG_EXTERNAL_DESCRIPTOR_HEAP;
		ZE_XESS_LOG_RET_FAILED(xessD3D12BuildPipelines(ctx, nullptr, false, initFlags), "Error building XeSS D3D12 pipelines!");

		// Init external descriptor pool
		xess_properties_t props = {};
		ZE_XESS_LOG_RET_FAILED(xessGetProperties(ctx, &outputRes, &props), "Error querity XeSS properties!");

		ZE_EXPECT_RET_FAILED_CODE(descInfo, dev.Get().dx12.AllocDescs(props.requiredDescriptorCount * Settings::GetBackbufferCount()));
		aliasBufferRegion = props.tempBufferHeapSize;
		aliasTextureRegion = props.tempTextureHeapSize;
		return {};
	}

	void XeSSInterface::FreeCtx(GFX::Device& dev) noexcept
	{
		ZE_ASSERT(IsInitialized(), "XeSS not initialized!");

		Destroy(false);
	}

	Status XeSSInterface::Execute(GFX::Device& dev, GFX::Pipeline::FrameBuffer& buffers, GFX::CommandList& cl,
		RID color, RID motionVectors, RID depth, RID exposure, RID responsive, RID output, const Float2& jitter, bool reset) const noexcept
	{
		ZE_ASSERT(IsInitialized(), "XeSS not initialized!");

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
		execParams.descriptorHeapOffset = descInfo.GPU.ptr - execParams.pDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr;
		execParams.descriptorHeapOffset += descSize * singleSetCount * Settings::GetCurrentBackbufferIndex();

		ZE_CODE_RET_FAILED(ZE_XESS_ERROR(xessD3D12Execute(ctx, cl.Get().dx12.GetList(), &execParams)));
		return {};
	}

	Status XeSSInterface::FinishInitialization(IHeap* buffHeap, U64 buffHeapOffset, IHeap* texHeap, U64 texHeapOffset) const noexcept
	{
		xess_d3d12_init_params_t initParams = {};
		initParams.outputResolution = outputRes;
		initParams.qualitySetting = qualityMode;
		initParams.initFlags = initFlags;
		initParams.creationNodeMask = 0;
		initParams.visibleNodeMask = 0;
		initParams.pTempBufferHeap = buffHeap;
		initParams.bufferHeapOffset = buffHeapOffset;
		initParams.pTempTextureHeap = texHeap;
		initParams.textureHeapOffset = texHeapOffset;
		initParams.pPipelineLibrary = nullptr;
		ZE_XESS_LOG_RET_FAILED(xessD3D12Init(ctx, &initParams), "Failed to initialize XeSS D3D12 context!");
		return {};
	}
}
#endif