#include "GFX/API/DX12/CommandList.h"
#include "GFX/API/DX/GraphicsException.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX12
{
	void CommandList::Open(Device& dev, ID3D12PipelineState* state)
	{
		ZE_GFX_ENABLE(dev);
		ZE_GFX_THROW_FAILED(commands->Reset(allocator.Get(), state));
	}

	CommandList::CommandList(GFX::Device& dev, CommandType type)
	{
		Init(dev.Get().dx12, type);
	}

	CommandList::CommandList(GFX::Device& dev)
	{
		ZE_GFX_ENABLE(dev.Get().dx12);
		ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));
		ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateCommandList1(0,
			D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commands)));
	}

	void CommandList::Open(GFX::Device& dev)
	{
		Open(dev.Get().dx12, nullptr);
	}

	void CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso)
	{
		Open(dev.Get().dx12, pso.Get().dx12.GetState());
	}

	void CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso)
	{
		Open(dev.Get().dx12, pso.Get().dx12.GetState());
	}

	void CommandList::SetState(GFX::Resource::PipelineStateCompute& pso)
	{
		commands->SetPipelineState(pso.Get().dx12.GetState());
	}

	void CommandList::SetState(GFX::Resource::PipelineStateGfx& pso)
	{
		commands->SetPipelineState(pso.Get().dx12.GetState());
	}

	void CommandList::Close(GFX::Device& dev)
	{
		Close(dev.Get().dx12);
	}

	void CommandList::Reset(GFX::Device& dev)
	{
		Reset(dev.Get().dx12);
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

	void CommandList::Init(Device& dev, CommandType type)
	{
		ZE_GFX_ENABLE(dev);
		ZE_GFX_THROW_FAILED(dev.GetDevice()->CreateCommandAllocator(GetCommandType(type), IID_PPV_ARGS(&allocator)));
		ZE_GFX_THROW_FAILED(dev.GetDevice()->CreateCommandList1(0,
			GetCommandType(type), D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commands)));
	}

	void CommandList::Open(Device& dev)
	{
		Open(dev, nullptr);
	}

	void CommandList::Close(Device& dev)
	{
		ZE_GFX_ENABLE(dev);
		ZE_GFX_THROW_FAILED(commands->Close());
	}

	void CommandList::Reset(Device& dev)
	{
		ZE_GFX_ENABLE(dev);
		ZE_GFX_THROW_FAILED(allocator->Reset());
	}
}