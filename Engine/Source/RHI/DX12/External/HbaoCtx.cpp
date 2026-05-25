#include "RHI/DX12/External/HbaoCtx.h"
#include "RHI/DX12/GarbageCollector.h"
#include "GFX/External/Error.h"

namespace ZE::RHI::DX12::External
{
	HbaoCtx::~HbaoCtx()
	{
		if (ctx)
			ctx->Release();
		if (srvDescInfo.Handle)
			GarbageCollector::Get().Register(GarbageCollector::Get().MarkInactive(srvDescInfo.Handle), std::move(srvDescInfo));
	}

	Expected<HbaoCtx> HbaoCtx::Create(GFX::Device& dev) noexcept
	{
		HbaoCtx hbao;

		auto& device = dev.Get().dx12;
		ZE_EXPECT_RET_FAILED(hbao.srvDescInfo, device.AllocDescs(GFSDK_SSAO_NUM_DESCRIPTORS_CBV_SRV_UAV_HEAP_D3D12));
		GarbageCollector::Get().MarkActive(device, hbao.srvDescInfo.Handle);

		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
		descHeapDesc.NodeMask = 1;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descHeapDesc.NumDescriptors = GFSDK_SSAO_NUM_DESCRIPTORS_RTV_HEAP_D3D12;
		ZE_DX_RET_FAILED_EXPECT(device.GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&hbao.rtvDescHeap)));

		GFSDK_SSAO_DescriptorHeaps_D3D12 descHeaps = {};
		descHeaps.CBV_SRV_UAV.pDescHeap = device.GetDescHeap();
		const U32 descSize = device.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		descHeaps.CBV_SRV_UAV.BaseIndex = Utils::SafeCast<U32>((hbao.srvDescInfo.GPU.ptr - device.GetDescHeap()->GetGPUDescriptorHandleForHeapStart().ptr) / descSize);
		descHeaps.RTV.pDescHeap = hbao.rtvDescHeap.Get();
		descHeaps.RTV.BaseIndex = 0;

		GFSDK_SSAO_CustomHeap customHeap = {};
		customHeap.delete_ = ::operator delete;
		customHeap.new_ = ::operator new;

		ZE_HBAO_LOG_RET_FAILED_EXPECT(ZE_HBAO_ERROR(GFSDK_SSAO_CreateContext_D3D12(device.GetDevice(), 1, descHeaps, &hbao.ctx, &customHeap)), "Failed to initialize HBAO+!");
		return hbao;
	}

	Status HbaoCtx::CreateResources(GFX::Device& dev, const GFSDK_SSAO_Parameters& params, UInt2 renderSize) noexcept
	{
		// Cmd queue is used mostly for resources cleanup so better to always use Main and see if it breaks things
		Status stat = ZE_HBAO_ERROR(ctx->PreCreateRTs(dev.Get().dx12.GetQueueMain(), params, renderSize.X, renderSize.Y));
		// Flush any warnings from HBAO+ initialization
		[[maybe_unused]] HRESULT hr = S_OK;
		ZE_DX_CHECK_DEBUG_INFO(hr);
		return stat;
	}

	Status HbaoCtx::Render(GFX::Device& dev, GFX::CommandList& cl, GFX::Pipeline::FrameBuffer& buffers, const GFSDK_SSAO_Parameters& params,
		RID depth, RID normals, RID output, const Float4x4& projection, const Float4x4* viewTps, bool blendMultiply, bool linearDepth) noexcept
	{
		auto& framebuff = buffers.Get().dx12;

		GFSDK_SSAO_InputData_D3D12 hbaoInput = {};
		hbaoInput.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
		std::memcpy(hbaoInput.DepthData.ProjectionMatrix.Data.Array, &projection, sizeof(Float4x4));
		hbaoInput.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
		hbaoInput.DepthData.MetersToViewSpaceUnits = 1.0f;
		hbaoInput.DepthData.Viewport.Enable = false; // Just default viewport
		hbaoInput.DepthData.FullResDepthTextureSRV.pResource = framebuff.GetResource(depth).Get();
		hbaoInput.DepthData.FullResDepthTextureSRV.GpuHandle = framebuff.GetSRV(depth).GpuShaderVisibleHandle.ptr;
		if (params.EnableDualLayerAO)
		{
			hbaoInput.DepthData.FullResDepthTexture2ndLayerSRV.pResource = framebuff.GetResource(depth).Get();
			hbaoInput.DepthData.FullResDepthTexture2ndLayerSRV.GpuHandle = framebuff.GetSRV(depth).GpuShaderVisibleHandle.ptr;
		}

		if (normals != INVALID_RID)
		{
			ZE_ASSERT(viewTps, "View matrix is required when normals are provided");

			hbaoInput.NormalData.Enable = true;
			std::memcpy(hbaoInput.NormalData.WorldToViewMatrix.Data.Array, viewTps, sizeof(Float4x4));
			hbaoInput.NormalData.WorldToViewMatrix.Layout = GFSDK_SSAO_COLUMN_MAJOR_ORDER;
			hbaoInput.NormalData.DecodeScale = 1.0f;
			hbaoInput.NormalData.DecodeBias = 0.0f;
			hbaoInput.NormalData.FullResNormalTextureSRV.pResource = framebuff.GetResource(normals).Get();
			hbaoInput.NormalData.FullResNormalTextureSRV.GpuHandle = framebuff.GetSRV(normals).GpuShaderVisibleHandle.ptr;
		}
		else
			hbaoInput.NormalData.Enable = false;

		GFSDK_SSAO_RenderTargetView_D3D12 outputRTV = {};
		outputRTV.pResource = framebuff.GetResource(output).Get();
		outputRTV.CpuHandle = framebuff.GetRTV(output).ptr;

		GFSDK_SSAO_Output_D3D12 hbaoOutput = {};
		hbaoOutput.pRenderTargetView = &outputRTV;
		hbaoOutput.Blend.Mode = blendMultiply ? GFSDK_SSAO_MULTIPLY_RGB : GFSDK_SSAO_OVERWRITE_RGB;

		return ZE_HBAO_ERROR(ctx->RenderAO(dev.Get().dx12.GetQueueMain(), cl.Get().dx12.GetList(), hbaoInput, params, hbaoOutput, linearDepth ? GFSDK_SSAO_DRAW_AO : GFSDK_SSAO_RENDER_AO));
	}
}