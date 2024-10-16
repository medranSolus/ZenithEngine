#include "RHI/DX12/SwapChain.h"
#include "RHI/DX12/DREDRecovery.h"

namespace ZE::RHI::DX12
{
	SwapChain::SwapChain(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx12);
		DX::ComPtr<DX::IFactory> factory = DX::CreateFactory(
#if _ZE_DEBUG_GFX_API
			debugManager
#endif
		);
		presentFlags = DX::CreateSwapChain(std::move(factory), dev.Get().dx12.GetQueueMain(), window.GetHandle(), swapChain, shaderInput
#if _ZE_DEBUG_GFX_API
			, debugManager
#endif
		);

		auto device = dev.Get().dx12.GetDevice();
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
		descHeapDesc.NodeMask = 0;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descHeapDesc.NumDescriptors = Settings::GetBackbufferCount();
		ZE_DX_THROW_FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&rtvDescHeap)));
		rtvSrv = new DescEntry[descHeapDesc.NumDescriptors];

		const U32 rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		const U32 srvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		srvHandle = dev.Get().dx12.AllocDescs(descHeapDesc.NumDescriptors);

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DX::GetDXFormat(Settings::BackbufferFormat);
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = rtvDesc.Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		for (U32 i = 0; i < descHeapDesc.NumDescriptors; ++i)
		{
			DX::ComPtr<IResource> buffer = nullptr;
			ZE_DX_THROW_FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&buffer)));
			ZE_DX_SET_ID(buffer, "Backbuffer " + std::to_string(i));
			ZE_DX_THROW_FAILED_INFO(device->CreateRenderTargetView(buffer.Get(), &rtvDesc, rtvHandle));
			rtvSrv[i].RTV = rtvHandle;
			if (shaderInput)
			{
				ZE_DX_THROW_FAILED_INFO(device->CreateShaderResourceView(buffer.Get(), &srvDesc, srvHandle.CPU));
				rtvSrv[i].SRVCpu = srvHandle.CPU;
				rtvSrv[i].SRVGpu = srvHandle.GPU;
			}
			else
				rtvSrv[i].SRVCpu.ptr = rtvSrv[i].SRVGpu.ptr = UINT64_MAX;
			rtvHandle.ptr += rtvDescSize;
			srvHandle.CPU.ptr += srvDescSize;
			srvHandle.GPU.ptr += srvDescSize;
		}
	}

	SwapChain::~SwapChain()
	{
		ZE_ASSERT_FREED(rtvDescHeap == nullptr && swapChain == nullptr && srvHandle.Handle == nullptr);
		if (rtvSrv)
			rtvSrv.DeleteArray();
	}

	void SwapChain::Present(GFX::Device& dev) const
	{
		ZE_DX_ENABLE(dev.Get().dx12);
		ZE_DX_SET_DEBUG_WATCH();
		if (FAILED(ZE_WIN_EXCEPT_RESULT = swapChain->Present(0, presentFlags)))
		{
			if (ZE_WIN_EXCEPT_RESULT == DXGI_ERROR_DEVICE_REMOVED)
			{
#if _ZE_DEBUG_GFX_API
				DREDRecovery::SaveDeviceRemovedData(dev.Get().dx12, "tdr_error.txt");
#endif
				throw ZE_DX_EXCEPT(dev.Get().dx12.GetDevice()->GetDeviceRemovedReason());
			}
			else
				throw ZE_DX_EXCEPT(ZE_WIN_EXCEPT_RESULT);
		}
	}

	void SwapChain::Free(GFX::Device& dev) noexcept
	{
		rtvDescHeap = nullptr;
		swapChain = nullptr;
		if (srvHandle.Handle)
			dev.Get().dx12.FreeDescs(srvHandle);
	}

	SwapChain::DescEntry SwapChain::GetCurrentBackbuffer(Device& dev, DX::ComPtr<IResource>& buffer)
	{
		ZE_DX_ENABLE(dev);
		const U32 current = Settings::GetCurrentBackbufferIndex();
		ZE_DX_THROW_FAILED(swapChain->GetBuffer(current, IID_PPV_ARGS(&buffer)));
		return rtvSrv[current];
	}
}