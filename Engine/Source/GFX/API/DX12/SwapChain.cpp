#include "GFX/API/DX12/SwapChain.h"
#include "GFX/API/DX12/DREDRecovery.h"

namespace ZE::GFX::API::DX12
{
	SwapChain::SwapChain(const Window::MainWindow& window, GFX::Device& dev, bool shaderInput)
	{
		ZE_DX_ENABLE(dev.Get().dx12);
		DX::ComPtr<IDXGIFactory7> factory = DX::CreateFactory(
#ifdef _ZE_MODE_DEBUG
			debugManager
#endif
		);
		presentFlags = DX::CreateSwapChain(std::move(factory), dev.Get().dx12.GetQueueMain(), window.GetHandle(), swapChain, shaderInput
#ifdef _ZE_MODE_DEBUG
			, debugManager
#endif
		);

		auto device = dev.Get().dx12.GetDevice();
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
		descHeapDesc.NodeMask = 0;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descHeapDesc.NumDescriptors = Settings::GetBackbufferCount();
		ZE_DX_THROW_FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&rtvDescHeap)));
		rtvSrv = new std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>[descHeapDesc.NumDescriptors];

		const U32 rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		const U32 srvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		auto srvHandle = dev.Get().dx12.AddStaticDescs(descHeapDesc.NumDescriptors);

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = DX::GetDXFormat(Settings::GetBackbufferFormat());
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = rtvDesc.Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		for (U32 i = 0; i < descHeapDesc.NumDescriptors; ++i)
		{
			DX::ComPtr<ID3D12Resource> buffer;
			ZE_DX_THROW_FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&buffer)));
			ZE_DX_THROW_FAILED_INFO(device->CreateRenderTargetView(buffer.Get(), &rtvDesc, rtvHandle));
			rtvSrv[i].first = rtvHandle;
			if (shaderInput)
			{
				ZE_DX_THROW_FAILED_INFO(device->CreateShaderResourceView(buffer.Get(), &srvDesc, srvHandle.first));
				rtvSrv[i].second = srvHandle.second;
			}
			else
				rtvSrv[i].second.ptr = -1;
			rtvHandle.ptr += rtvDescSize;
			srvHandle.first.ptr += srvDescSize;
			srvHandle.second.ptr += srvDescSize;
		}

		presentBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		presentBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		presentBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		presentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		presentBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	}

	SwapChain::~SwapChain()
	{
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
#ifdef _ZE_MODE_DEBUG
				DREDRecovery::SaveDeviceRemovedData(dev.Get().dx12, "tdr_error.txt");
#endif
				throw ZE_DX_EXCEPT(dev.Get().dx12.GetDevice()->GetDeviceRemovedReason());
			}
			else
				throw ZE_DX_EXCEPT(ZE_WIN_EXCEPT_RESULT);
		}
	}

	void SwapChain::PrepareBackbuffer(GFX::Device& dev, GFX::CommandList& cl) const
	{
		cl.Get().dx12.Open(dev.Get().dx12);
		cl.Get().dx12.GetList()->ResourceBarrier(1, &presentBarrier);
		cl.Get().dx12.Close(dev.Get().dx12);
		dev.Get().dx12.ExecuteMain(cl);
	}

	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> SwapChain::SetCurrentBackbuffer(GFX::Device& dev, DX::ComPtr<ID3D12Resource>& buffer)
	{
		ZE_DX_ENABLE(dev.Get().dx12);
		const U32 current = Settings::GetCurrentBackbufferIndex();
		ZE_DX_THROW_FAILED(swapChain->GetBuffer(current, IID_PPV_ARGS(&buffer)));
		presentBarrier.Transition.pResource = buffer.Get();
		return rtvSrv[current];
	}
}