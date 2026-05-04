#include "RHI/DX12/SwapChain.h"
#include "RHI/DX12/DREDRecovery.h"

namespace ZE::RHI::DX12
{
	Expected<SwapChain> SwapChain::Create(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput) noexcept
	{
		DX::ComPtr<DX::IFactory> factory = nullptr;
		ZE_EXPECT_RET_FAILED(factory, DX::CreateFactory());

		SwapChain swapChain = {};
		swapChain.srcDev = &dev.Get().dx12;
		ZE_EXPECT_RET_FAILED(swapChain.swapChain, DX::CreateSwapChain(std::move(factory), dev.Get().dx12.GetQueueMain(), window.GetHandle(), shaderInput, swapChain.presentFlags));

		auto device = dev.Get().dx12.GetDevice();
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
		descHeapDesc.NodeMask = 0;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descHeapDesc.NumDescriptors = Settings::GetBackbufferCount();
		ZE_DX_RET_FAILED_EXPECT(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&swapChain.rtvDescHeap)));
		swapChain.rtvSrv = std::make_unique_for_overwrite<DescEntry[]>(descHeapDesc.NumDescriptors);

		const U32 rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		const U32 srvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain.rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		if (shaderInput)
		{
			ZE_EXPECT_RET_FAILED(swapChain.srvHandle, dev.Get().dx12.AllocDescs(descHeapDesc.NumDescriptors));
		}

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
			ZE_DX_RET_FAILED_EXPECT(swapChain.swapChain->GetBuffer(i, IID_PPV_ARGS(&buffer)));
			ZE_DX_SET_ID(buffer, "Backbuffer_" + std::to_string(i));
			ZE_DX_RET_FAILED_DEBUG_EXPECT(device->CreateRenderTargetView(buffer.Get(), &rtvDesc, rtvHandle));
			swapChain.rtvSrv[i].RTV = rtvHandle;
			if (shaderInput)
			{
				ZE_DX_RET_FAILED_DEBUG_EXPECT(device->CreateShaderResourceView(buffer.Get(), &srvDesc, swapChain.srvHandle.CPU));
				swapChain.rtvSrv[i].SRVCpu = swapChain.srvHandle.CPU;
				swapChain.rtvSrv[i].SRVGpu = swapChain.srvHandle.GPU;
				swapChain.srvHandle.CPU.ptr += srvDescSize;
				swapChain.srvHandle.GPU.ptr += srvDescSize;
			}
			else
				swapChain.rtvSrv[i].SRVCpu.ptr = swapChain.rtvSrv[i].SRVGpu.ptr = UINT64_MAX;
			rtvHandle.ptr += rtvDescSize;
		}
	}

	SwapChain::~SwapChain()
	{
		if (srvHandle.Handle)
		{
			ZE_ASSERT(srcDev, "No source Device for cleanup!");
			srcDev->FreeDescs(srvHandle);
		}
	}

	Status SwapChain::Present(GFX::Device& dev) const noexcept
	{
		HRESULT hr = swapChain->Present(0, presentFlags);
		if (FAILED(hr))
		{
			if (hr == DXGI_ERROR_DEVICE_REMOVED)
			{
#if _ZE_DEBUG_GFX_API
				DREDRecovery::SaveDeviceRemovedData(dev.Get().dx12, std::string(Logger::LOG_DIR) + "tdr_error.txt");
#endif
				hr = dev.Get().dx12.GetDevice()->GetDeviceRemovedReason();
			}
			return DX::Error::Make(hr);
		}
		return {};
	}

	Expected<SwapChain::DescEntry> SwapChain::GetCurrentBackbuffer(Device& dev, DX::ComPtr<IResource>& buffer) noexcept
	{
		const U32 current = Settings::GetCurrentBackbufferIndex();
		ZE_DX_RET_FAILED_EXPECT(swapChain->GetBuffer(current, IID_PPV_ARGS(&buffer)));
		return rtvSrv[current];
	}
}