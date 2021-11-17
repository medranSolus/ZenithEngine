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
		Init(dev.Get().dx12, CommandType::All);
	}

	CommandList::~CommandList()
	{
		if (barriers != nullptr)
			Table::Clear(barriersInfo.Size == 0 ? barriersInfo.Allocated : barriersInfo.Size, barriers);
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

	void CommandList::FinishBarriers() noexcept
	{
		if (barriersInfo.Size != 0)
			commands->ResourceBarrier(barriersInfo.Size, barriers);
	}

	void CommandList::Init(Device& dev, CommandType type)
	{
		ZE_GFX_ENABLE_ID(dev);
		ZE_GFX_THROW_FAILED(dev.GetDevice()->CreateCommandAllocator(GetCommandType(type), IID_PPV_ARGS(&allocator)));
		ZE_GFX_THROW_FAILED(dev.GetDevice()->CreateCommandList1(0,
			GetCommandType(type), D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commands)));
		switch (type)
		{
		default:
		case ZE::GFX::CommandType::All:
		{
			ZE_GFX_SET_ID(allocator, "direct_allocator");
			ZE_GFX_SET_ID(commands, "direct_command");
			break;
		}
		case ZE::GFX::CommandType::Bundle:
		{
			ZE_GFX_SET_ID(allocator, "bundle_allocator");
			ZE_GFX_SET_ID(commands, "bundle_command");
			break;
		}
		case ZE::GFX::CommandType::Compute:
		{
			ZE_GFX_SET_ID(allocator, "compute_allocator");
			ZE_GFX_SET_ID(commands, "compute_command");
			break;
		}
		case ZE::GFX::CommandType::Copy:
		{
			ZE_GFX_SET_ID(allocator, "copy_allocator");
			ZE_GFX_SET_ID(commands, "copy_command");
			break;
		}
		}
		barriersInfo.Size = 0;
		barriersInfo.Allocated = BARRIER_LIST_GROW_SIZE;
		barriers = Table::Create<D3D12_RESOURCE_BARRIER>(BARRIER_LIST_GROW_SIZE);
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
		Table::Resize(barriersInfo, barriers, BARRIER_LIST_GROW_SIZE);
		barriersInfo.Size = 0;
	}

	void CommandList::AddBarrierTransition(ID3D12Resource* resource, GFX::Resource::State before,
		GFX::Resource::State after, GFX::Pipeline::BarrierType type) noexcept
	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = GetTransitionType(type);
		barrier.Transition.pResource = resource;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = GetResourceState(before);
		barrier.Transition.StateAfter = GetResourceState(after);
		Table::Append<BARRIER_LIST_GROW_SIZE>(barriersInfo, barriers, std::move(barrier));
	}

	void CommandList::AddBarrierAliasing(ID3D12Resource* before, ID3D12Resource* after, GFX::Pipeline::BarrierType type) noexcept
	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
		barrier.Flags = GetTransitionType(type);
		barrier.Aliasing.pResourceBefore = before;
		barrier.Aliasing.pResourceAfter = after;
		Table::Append<BARRIER_LIST_GROW_SIZE>(barriersInfo, barriers, std::move(barrier));
	}

	void CommandList::AddBarrierUAV(ID3D12Resource* resource, GFX::Pipeline::BarrierType type) noexcept
	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags = GetTransitionType(type);
		barrier.UAV.pResource = resource;
		Table::Append<BARRIER_LIST_GROW_SIZE>(barriersInfo, barriers, std::move(barrier));
	}
}