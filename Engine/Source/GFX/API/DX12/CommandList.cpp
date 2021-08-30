#include "GFX/API/DX12/CommandList.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12
{
	void CommandList::Reset(Device& dev, ID3D12PipelineState* state)
	{
		ZE_GFX_ENABLE(dev);
		ZE_GFX_THROW_FAILED(allocator->Reset());
		ZE_GFX_THROW_FAILED(commands->Reset(allocator.Get(), state));
	}

	CommandList::CommandList(GFX::Device& dev)
	{
		ZE_GFX_ENABLE(dev.Get().dx12);
		ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));
		ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateCommandList(0,
			D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&commands)));
	}

	CommandList::CommandList(GFX::Device& dev, CommandType type)
	{
		ZE_GFX_ENABLE(dev.Get().dx12);
		ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateCommandAllocator(GetCommandType(type), IID_PPV_ARGS(&allocator)));
		ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateCommandList(0,
			GetCommandType(type), allocator.Get(), nullptr, IID_PPV_ARGS(&commands)));
	}

	void CommandList::Close(GFX::Device& dev)
	{
		ZE_GFX_ENABLE(dev.Get().dx12);
		ZE_GFX_THROW_FAILED(commands->Close());
	}

	void CommandList::DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx12);
		ZE_GFX_THROW_FAILED_INFO(commands->DrawIndexedInstanced(count, 1, 0, 0, 0));
	}

	void CommandList::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx12);
		ZE_GFX_THROW_FAILED_INFO(commands->Dispatch(groupX, groupY, groupZ));
	}
}