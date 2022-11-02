#include "GFX/API/DX12/GPerf.h"

namespace ZE::GFX::API::DX12
{
	GPerf::GPerf(GFX::Device& dev)
	{
		ZE_DX_ENABLE(dev.Get().dx12);

		D3D12_QUERY_HEAP_DESC desc;
		desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		desc.Count = 2;
		desc.NodeMask = 0;
		ZE_DX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateQueryHeap(&desc, IID_PPV_ARGS(&queryHeap)));

		D3D12_HEAP_PROPERTIES dataHeapDesc;
		dataHeapDesc.Type = D3D12_HEAP_TYPE_READBACK;
		dataHeapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		dataHeapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		dataHeapDesc.CreationNodeMask = 0;
		dataHeapDesc.VisibleNodeMask = 0;
		D3D12_RESOURCE_DESC dataDesc = dev.Get().dx12.GetBufferDesc(sizeof(U64) * 2);
		ZE_DX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateCommittedResource(&dataHeapDesc,
			D3D12_HEAP_FLAG_NONE, &dataDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&data)));
	}

	void GPerf::Start(GFX::CommandList& cl) noexcept
	{
		cl.Get().dx12.GetList()->EndQuery(queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0);
		listType = cl.Get().dx12.GetList()->GetType();
	}

	void GPerf::Stop(GFX::CommandList& cl) const noexcept
	{
		cl.Get().dx12.GetList()->EndQuery(queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 1);
		cl.Get().dx12.GetList()->ResolveQueryData(queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP,
			0, 2, data.Get(), 0);
	}

	long double GPerf::GetData(GFX::Device& dev) noexcept
	{
		U64 frequency = 0;
		switch (listType)
		{
		default:
		case D3D12_COMMAND_LIST_TYPE_BUNDLE:
			ZE_ENUM_UNHANDLED();
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
		{
			dev.Get().dx12.GetQueueMain()->GetTimestampFrequency(&frequency);
			break;
		}
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		{
			dev.Get().dx12.GetQueueCompute()->GetTimestampFrequency(&frequency);
			break;
		}
		case D3D12_COMMAND_LIST_TYPE_COPY:
		{
			dev.Get().dx12.GetQueueCopy()->GetTimestampFrequency(&frequency);
			break;
		}
		}
		long double megaFrequency = static_cast<long double>(frequency) / 1000000.0L;
		U64* timestamps = nullptr;
		data->Map(0, nullptr, reinterpret_cast<void**>(&timestamps));
		megaFrequency = static_cast<long double>(timestamps[1] - timestamps[0]) / megaFrequency;
		data->Unmap(0, nullptr);
		return megaFrequency;
	}
}