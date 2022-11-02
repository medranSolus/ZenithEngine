#include "GFX/API/DX12/CommandList.h"
#include "GFX/Binding/Schema.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX12
{
	void CommandList::Open(Device& dev, ID3D12PipelineState* state)
	{
		ZE_DX_ENABLE(dev);
		ZE_DX_THROW_FAILED(commands->Reset(allocator.Get(), state));
		ID3D12DescriptorHeap* heaps[] = { dev.GetDescHeap() };
		commands->SetDescriptorHeaps(1, heaps);
	}

	CommandList::CommandList(GFX::Device& dev, CommandType type)
	{
		Init(dev.Get().dx12, type);
	}

	CommandList::CommandList(GFX::Device& dev)
	{
		Init(dev.Get().dx12, CommandType::All);
	}

#if _ZE_GFX_MARKERS
	void CommandList::TagBegin(GFX::Device& dev, const wchar_t* tag, Pixel color) const noexcept
	{
		switch (Settings::GetGpuVendor())
		{
		case VendorGPU::AMD:
		{
			agsDriverExtensionsDX12_PushMarker(dev.Get().dx12.GetAGSContext(), commands.Get(), Utils::ToAscii(tag).c_str());
			break;
		}
		}
		PIXBeginEvent(commands.Get(), PIX_COLOR(color.Red, color.Blue, color.Green), tag);
	}

	void CommandList::TagEnd(GFX::Device& dev) const noexcept
	{
		switch (Settings::GetGpuVendor())
		{
		case VendorGPU::AMD:
		{
			agsDriverExtensionsDX12_PopMarker(dev.Get().dx12.GetAGSContext(), commands.Get());
			break;
		}
		}
		PIXEndEvent(commands.Get());
	}
#endif

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

	void CommandList::Draw(GFX::Device& dev, U32 vertexCount) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		ZE_DX_ENABLE_INFO(dev.Get().dx12);
		ZE_DX_THROW_FAILED_INFO(commands->DrawInstanced(vertexCount, 1, 0, 0));
	}

	void CommandList::DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		ZE_DX_ENABLE_INFO(dev.Get().dx12);
		ZE_DX_THROW_FAILED_INFO(commands->DrawIndexedInstanced(indexCount, 1, 0, 0, 0));
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

	void CommandList::Init(Device& dev, CommandType type)
	{
		ZE_DX_ENABLE_ID(dev);
		ZE_DX_THROW_FAILED(dev.GetDevice()->CreateCommandAllocator(GetCommandType(type), IID_PPV_ARGS(&allocator)));
		ZE_DX_THROW_FAILED(dev.GetDevice()->CreateCommandList1(0,
			GetCommandType(type), D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commands)));

#if _ZE_DEBUG_GFX_API
		switch (type)
		{
		default:
		case ZE::GFX::CommandType::All:
		{
			ZE_DX_SET_ID(allocator, "direct_allocator");
			ZE_DX_SET_ID(commands, "direct_command");
			break;
		}
		case ZE::GFX::CommandType::Bundle:
		{
			ZE_DX_SET_ID(allocator, "bundle_allocator");
			ZE_DX_SET_ID(commands, "bundle_command");
			break;
		}
		case ZE::GFX::CommandType::Compute:
		{
			ZE_DX_SET_ID(allocator, "compute_allocator");
			ZE_DX_SET_ID(commands, "compute_command");
			break;
		}
		case ZE::GFX::CommandType::Copy:
		{
			ZE_DX_SET_ID(allocator, "copy_allocator");
			ZE_DX_SET_ID(commands, "copy_command");
			break;
		}
		}
#endif
	}

	void CommandList::Open(Device& dev)
	{
		Open(dev, nullptr);
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