#include "RHI/DX12/Resource/DynamicCBuffer.h"

namespace ZE::RHI::DX12::Resource
{
	Status DynamicCBuffer::AllocBlock(GFX::Device& dev) noexcept
	{
		auto& device = dev.Get().dx12;

		const D3D12_RESOURCE_DESC1 desc = dev.Get().dx12.GetBufferDesc(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
		ResourceInfo resource = {};
		ZE_EXPECT_RET_FAILED_CODE(resource, device.CreateBuffer(desc, true));
		GarbageCollector::Get().MarkActive(device, resource.Handle);
		ZE_DX_SET_ID(resource.Resource, "DynamicCBuffer_" + std::to_string(resInfo.size()));

		const D3D12_RANGE range = {};
		ZE_DX_RET_FAILED(resource.Resource->Map(0, &range, reinterpret_cast<void**>(&buffer)));

		const D3D12_GPU_VIRTUAL_ADDRESS address = resource.Resource->GetGPUVirtualAddress();
		resInfo.emplace_back(std::move(resource), address);
		return {};
	}

	Status DynamicCBuffer::MapBlock(GFX::Device& dev, U64 block) noexcept
	{
		ZE_ASSERT(block < resInfo.size(), "Trying to map block outside of range!");

		const D3D12_RANGE range = { 0 };
		ZE_DX_RET_FAILED(resInfo.at(block).first.Resource->Map(0, &range, reinterpret_cast<void**>(&buffer)));
		return {};
	}

	DynamicCBuffer::~DynamicCBuffer()
	{
		if (buffer)
			resInfo.at(currentBlock).first.Resource->Unmap(0, nullptr);
		for (auto& res : resInfo)
			GarbageCollector::Get().RegisterDynamicBuffer(GarbageCollector::Get().MarkInactive(res.first.Handle), std::move(res.first));
	}

	Expected<DynamicCBuffer> DynamicCBuffer::Create(GFX::Device& dev) noexcept
	{
		DynamicCBuffer buffer = {};
		if (Status code = buffer.AllocBlock(dev))
			return std::unexpected(code);
		return buffer;
	}

	Expected<GFX::Resource::DynamicBufferAlloc> DynamicCBuffer::Alloc(GFX::Device& dev, const void* values, U32 bytes) noexcept
	{
		ZE_ASSERT(buffer, "Dynamic buffer has been freed already!");
		ZE_ASSERT(bytes <= D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, "Structure too large for dynamic buffer!");

		const U32 newBlock = Math::AlignUp(bytes, 256U);
#if !_ZE_RENDER_GRAPH_SINGLE_THREAD
		LockGuardRW lock(allocLock);
#endif
		if (nextOffset + newBlock > D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
		{
			nextOffset = 0;
			resInfo.at(currentBlock).first.Resource->Unmap(0, nullptr);
			Status code = {};
			if (++currentBlock >= resInfo.size())
				code = AllocBlock(dev);
			else
				code = MapBlock(dev, currentBlock);
			if (code)
				return std::unexpected(code);
		}
		std::memcpy(buffer + nextOffset, values, bytes);

		GFX::Resource::DynamicBufferAlloc info
		{
			nextOffset, currentBlock
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
		{
			ZE_DX_CHECK_FAILED(list->SetComputeRootConstantBufferView(bindCtx.Count++, address), "Setting compute dynamic CBV resulted in debug layer messages!");
		}
		else
		{
			ZE_DX_CHECK_FAILED(list->SetGraphicsRootConstantBufferView(bindCtx.Count++, address), "Setting GFX dynamic CBV resulted in debug layer messages!");
		}
	}

	Status DynamicCBuffer::StartFrame(GFX::Device& dev) noexcept
	{
		ZE_ASSERT(buffer, "Dynamic buffer has been freed already!");

		nextOffset = 0;
		const U64 blockCount = resInfo.size();
		if (blockCount > 1)
		{
			resInfo.at(currentBlock).first.Resource->Unmap(0, nullptr);
			if (Status code = MapBlock(dev, 0))
				return code;

			if (currentBlock + BLOCK_SHRINK_STEP < blockCount)
			{
				for (U64 i = currentBlock + 1; i < blockCount; ++i)
					GarbageCollector::Get().RegisterDynamicBuffer(GarbageCollector::Get().MarkInactive(resInfo.at(i).first.Handle), std::move(resInfo.at(i).first));
				resInfo.resize(currentBlock + 1);
			}
			currentBlock = 0;
		}
		return {};
	}
}