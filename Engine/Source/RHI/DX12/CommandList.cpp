#include "RHI/DX12/CommandList.h"
#include "GFX/Resource/Generic.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "GFX/Device.h"

namespace ZE::RHI::DX12
{
	void CommandList::Open(Device& dev, IPipelineState* state)
	{
		ZE_DX_ENABLE(dev);
		ZE_DX_THROW_FAILED(commands->Reset(allocator.Get(), state));
		IDescriptorHeap* heaps[] = { dev.GetDescHeap() };
		commands->SetDescriptorHeaps(1, heaps);
	}

	CommandList::CommandList(GFX::Device& dev, GFX::QueueType type)
	{
		Init(dev.Get().dx12, type);
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
		commands->IASetPrimitiveTopology(pso.Get().dx12.GetTopology());
	}

	void CommandList::Close(GFX::Device& dev)
	{
		Close(dev.Get().dx12);
	}

	void CommandList::Reset(GFX::Device& dev)
	{
		Reset(dev.Get().dx12);
	}

	void CommandList::Barrier(GFX::Device& dev, GFX::Resource::GenericResourceBarrier* barriers, U32 count) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		ZE_DX_ENABLE_INFO(dev.Get().dx12);

		std::unique_ptr<D3D12_RESOURCE_BARRIER[]> resBarriers = std::make_unique<D3D12_RESOURCE_BARRIER[]>(count);
		for (U32 i = 0; i < count; ++i)
		{
			ZE_ASSERT(barriers[i].Resource, "Empty Generic resource from barrier!");

			resBarriers[i].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			if (barriers[i].IsUAV)
			{
				resBarriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				resBarriers[i].UAV.pResource = barriers[i].Resource->Get().dx12.GetResource();
			}
			else
			{
				resBarriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				resBarriers[i].Transition.pResource = barriers[i].Resource->Get().dx12.GetResource();
				resBarriers[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				resBarriers[i].Transition.StateBefore = GetResourceState(barriers[i].Before);
				resBarriers[i].Transition.StateAfter = GetResourceState(barriers[i].After);
			}
		}
		ZE_DX_THROW_FAILED_INFO(commands->ResourceBarrier(count, resBarriers.get()));
	}

	void CommandList::DrawFullscreen(GFX::Device& dev) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		ZE_DX_ENABLE_INFO(dev.Get().dx12);

		commands->IASetVertexBuffers(0, 0, nullptr);
		commands->IASetIndexBuffer(nullptr);
		ZE_DX_THROW_FAILED_INFO(commands->DrawInstanced(3, 1, 0, 0));
	}

	void CommandList::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		ZE_DX_ENABLE_INFO(dev.Get().dx12);
		ZE_DX_THROW_FAILED_INFO(commands->Dispatch(groupX, groupY, groupZ));
	}

#if _ZE_GFX_MARKERS
	void CommandList::TagBegin(GFX::Device& dev, std::string_view tag, Pixel color) const noexcept
	{
		switch (Settings::GpuVendor)
		{
		case GFX::VendorGPU::AMD:
		{
			agsDriverExtensionsDX12_PushMarker(dev.Get().dx12.GetAGSContext(), commands.Get(), tag.data());
			break;
		}
		default:
			break;
		}
		PIXBeginEvent(commands.Get(), PIX_COLOR(color.Red, color.Blue, color.Green), tag.data());
	}

	void CommandList::TagEnd(GFX::Device& dev) const noexcept
	{
		switch (Settings::GpuVendor)
		{
		case GFX::VendorGPU::AMD:
		{
			agsDriverExtensionsDX12_PopMarker(dev.Get().dx12.GetAGSContext(), commands.Get());
			break;
		}
		default:
			break;
		}
		PIXEndEvent(commands.Get());
	}
#endif

	void CommandList::Init(Device& dev, GFX::QueueType type)
	{
		ZE_DX_ENABLE_ID(dev);
		ZE_DX_THROW_FAILED(dev.GetDevice()->CreateCommandAllocator(GetCommandType(type), IID_PPV_ARGS(&allocator)));
		ZE_DX_THROW_FAILED(dev.GetDevice()->CreateCommandList1(0,
			GetCommandType(type), D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commands)));

#if _ZE_DEBUG_GFX_NAMES
		switch (type)
		{
		default:
		case GFX::QueueType::Main:
		{
			ZE_DX_SET_ID(allocator, "direct_allocator");
			ZE_DX_SET_ID(commands, "direct_command");
			break;
		}
		case GFX::QueueType::Compute:
		{
			ZE_DX_SET_ID(allocator, "compute_allocator");
			ZE_DX_SET_ID(commands, "compute_command");
			break;
		}
		case GFX::QueueType::Copy:
		{
			ZE_DX_SET_ID(allocator, "copy_allocator");
			ZE_DX_SET_ID(commands, "copy_command");
			break;
		}
		}
#endif
	}

	void CommandList::Close(Device& dev)
	{
		ZE_DX_ENABLE(dev);
		ZE_DX_THROW_FAILED(commands->Close());
	}

	void CommandList::Reset(Device& dev)
	{
		ZE_DX_ENABLE(dev);
		ZE_DX_THROW_FAILED(allocator->Reset());
	}
}