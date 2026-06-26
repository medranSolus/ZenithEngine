#include "RHI/DX12/CommandList.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "GFX/Device.h"

namespace ZE::RHI::DX12
{
	Status CommandList::Open(Device& dev, IPipelineState* state) const noexcept
	{
		ZE_DX_RET_FAILED(commands->Reset(allocator.Get(), state));
		RestoreExternalState(dev);
		return {};
	}

	void CommandList::RestoreExternalState(Device& dev) const noexcept
	{
		IDescriptorHeap* heaps[] = { dev.GetDescHeap() };
		commands->SetDescriptorHeaps(1, heaps);
	}

	Expected<CommandList> CommandList::Create(GFX::Device& dev, GFX::QueueType type) noexcept
	{
		return Create(dev.Get().dx12, type);
	}

	Status CommandList::Open(GFX::Device& dev) const noexcept
	{
		return Open(dev.Get().dx12, nullptr);
	}

	Status CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso) const noexcept
	{
		return Open(dev.Get().dx12, pso.Get().dx12.GetState());
	}

	Status CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso) const noexcept
	{
		if (Status code = Open(dev.Get().dx12, pso.Get().dx12.GetState()))
			return code;
		commands->IASetPrimitiveTopology(pso.Get().dx12.GetTopology());
		return {};
	}

	void CommandList::RestoreExternalState(GFX::Device& dev) const noexcept
	{
		RestoreExternalState(dev.Get().dx12);
	}

	Status CommandList::Close(GFX::Device& dev) noexcept
	{
		return Close(dev.Get().dx12);
	}

	Status CommandList::Reset(GFX::Device& dev) noexcept
	{
		return Reset(dev.Get().dx12);
	}

	void CommandList::DrawFullscreen(GFX::Device& dev) const noexcept
	{
		commands->IASetVertexBuffers(0, 0, nullptr);
		commands->IASetIndexBuffer(nullptr);
		ZE_DX_CHECK_FAILED(commands->DrawInstanced(3, 1, 0, 0), "Fullscreen draw produced debug layer messages!");
	}

	void CommandList::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept
	{
		ZE_DX_CHECK_FAILED(commands->Dispatch(groupX, groupY, groupZ), "Dispatch produced debug layer messages!");
	}

	void CommandList::WriteBreadcrumbs(GFX::Device& dev, U32 value, U64 location, void* breadcrumbsBuffer, bool isBegin) const noexcept
	{
		const D3D12_WRITEBUFFERIMMEDIATE_MODE mode = isBegin ? D3D12_WRITEBUFFERIMMEDIATE_MODE_MARKER_IN : D3D12_WRITEBUFFERIMMEDIATE_MODE_MARKER_OUT;
		const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER params = { location, value };

		ZE_DX_CHECK_FAILED(commands->WriteBufferImmediate(1, &params, &mode), "Breadcrumb call produced debug layer messages!");
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

	Expected<CommandList> CommandList::Create(Device& dev, GFX::QueueType type) noexcept
	{
		CommandList cl;
		ZE_DX_RET_FAILED_EXPECT(dev.GetDevice()->CreateCommandAllocator(GetCommandType(type), IID_PPV_ARGS(&cl.allocator)));
		ZE_DX_RET_FAILED_EXPECT(dev.GetDevice()->CreateCommandList1(0,
			GetCommandType(type), D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&cl.commands)));

#if _ZE_DEBUG_GFX_NAMES
		switch (type)
		{
		default:
		case GFX::QueueType::Main:
		{
			ZE_DX_SET_ID(cl.allocator, "direct_allocator");
			ZE_DX_SET_ID(cl.commands, "direct_command");
			break;
		}
		case GFX::QueueType::Compute:
		{
			ZE_DX_SET_ID(cl.allocator, "compute_allocator");
			ZE_DX_SET_ID(cl.commands, "compute_command");
			break;
		}
		case GFX::QueueType::Copy:
		{
			ZE_DX_SET_ID(cl.allocator, "copy_allocator");
			ZE_DX_SET_ID(cl.commands, "copy_command");
			break;
		}
		}
#endif
		return cl;
	}

	Status CommandList::Close(Device& dev) noexcept
	{
		ZE_DX_RET_FAILED(commands->Close());
		return {};
	}

	Status CommandList::Reset(Device& dev) const noexcept
	{
		ZE_DX_RET_FAILED(allocator->Reset());
		return {};
	}
}