#include "RHI/DX12/Resource/DynamicCBuffer.h"

namespace ZE::RHI::DX12::Resource
{
	void DynamicCBuffer::AllocBlock(GFX::Device& dev)
	{
		auto& device = dev.Get().dx12;
		ZE_DX_ENABLE_ID(device);

		const D3D12_RESOURCE_DESC1 desc = dev.Get().dx12.GetBufferDesc(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
		ResourceInfo resource = device.CreateBuffer(desc, true);
		ZE_DX_SET_ID(resource.Resource, "DynamicCBuffer_" + std::to_string(resInfo.size()));

		const D3D12_RANGE range = {};
		ZE_DX_THROW_FAILED(resource.Resource->Map(0, &range, reinterpret_cast<void**>(&buffer)));

		const D3D12_GPU_VIRTUAL_ADDRESS address = resource.Resource->GetGPUVirtualAddress();
		resInfo.emplace_back(std::move(resource), address);
	}

	void DynamicCBuffer::MapBlock(GFX::Device& dev, U64 block)
	{
		ZE_ASSERT(block < resInfo.size(), "Trying to map block outside of range!");
		ZE_DX_ENABLE(dev.Get().dx12);

		const D3D12_RANGE range = { 0 };
		ZE_DX_THROW_FAILED(resInfo.at(block).first.Resource->Map(0, &range, reinterpret_cast<void**>(&buffer)));
	}

	DynamicCBuffer::~DynamicCBuffer()
	{
		for ([[maybe_unused]] auto& res : resInfo)
		{
			ZE_ASSERT(res.first.IsFree(), "Resource not freed before deletion!");
		}
	}

	GFX::Resource::DynamicBufferAlloc DynamicCBuffer::Alloc(GFX::Device& dev, const void* values, U32 bytes)
	{
		ZE_ASSERT(buffer, "Dynamic buffer has been freed already!");
		ZE_ASSERT(bytes <= D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, "Structure too large for dynamic buffer!");

		const U32 newBlock = Math::AlignUp(bytes, 256U);
#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
		const std::lock_guard<std::mutex> lock(allocLock);
#endif
		if (nextOffset + newBlock > D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
		{
			nextOffset = 0;
			resInfo.at(currentBlock).first.Resource->Unmap(0, nullptr);
			if (++currentBlock >= resInfo.size())
				AllocBlock(dev);
			else
				MapBlock(dev, currentBlock);
		}
		memcpy(buffer + nextOffset, values, bytes);

		GFX::Resource::DynamicBufferAlloc info
		{
			nextOffset,
				currentBlock
		};
		nextOffset += newBlock;
		return info;
	}

	void DynamicCBuffer::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, const GFX::Resource::DynamicBufferAlloc& allocInfo) const noexcept
	{
		ZE_ASSERT(buffer, "Dynamic buffer has been freed already!");
		ZE_ASSERT(allocInfo.Block <= currentBlock, "Block out of range!");
		ZE_ASSERT(allocInfo.Offset < D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, "Offset out of range!");

		const auto& schema = bindCtx.BindingSchema.Get().dx12;
		ZE_ASSERT(schema.GetCurrentType(bindCtx.Count) == Binding::Schema::BindType::CBV,
			"Bind slot is not a constant buffer! Wrong root signature or order of bindings!");

		const D3D12_GPU_VIRTUAL_ADDRESS address = resInfo.at(allocInfo.Block).second + allocInfo.Offset;
		auto* list = cl.Get().dx12.GetList();
		if (schema.IsCompute())
			list->SetComputeRootConstantBufferView(bindCtx.Count++, address);
		else
			list->SetGraphicsRootConstantBufferView(bindCtx.Count++, address);
	}

	void DynamicCBuffer::StartFrame(GFX::Device& dev)
	{
		ZE_ASSERT(buffer, "Dynamic buffer has been freed already!");

		nextOffset = 0;
		const U64 blockCount = resInfo.size();
		if (blockCount > 1)
		{
			resInfo.at(currentBlock).first.Resource->Unmap(0, nullptr);
			MapBlock(dev, 0);

			if (currentBlock + BLOCK_SHRINK_STEP < blockCount)
			{
				for (U64 i = currentBlock + 1; i < blockCount; ++i)
					dev.Get().dx12.FreeDynamicBuffer(resInfo.at(i).first);
				resInfo.resize(currentBlock + 1);
			}
			currentBlock = 0;
		}
	}

	void DynamicCBuffer::Free(GFX::Device& dev) noexcept
	{
		ZE_ASSERT(buffer, "Dynamic buffer has been freed already!");

		resInfo.back().first.Resource->Unmap(0, nullptr);
		buffer = nullptr;
		for (auto& res : resInfo)
			dev.Get().dx12.FreeDynamicBuffer(res.first);
	}
}