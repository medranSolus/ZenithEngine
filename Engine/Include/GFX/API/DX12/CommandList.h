#pragma once
#include "D3D12.h"
#include "Table.h"
ZE_WARNING_PUSH
#include "WinPixEventRuntime/pix3.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	class Device;
	namespace Resource
	{
		class PipelineStateCompute;
		class PipelineStateGfx;
	}
}
namespace ZE::GFX::API::DX12
{
	class Device;

	class CommandList final
	{
		DX::ComPtr<IGraphicsCommandList> commands;
		DX::ComPtr<ICommandAllocator> allocator;

		void Open(Device& dev, IPipelineState* state);

	public:
		CommandList() = default;
		CommandList(GFX::Device& dev) : CommandList(dev, QueueType::Main) {}
		CommandList(GFX::Device& dev, QueueType type);
		ZE_CLASS_MOVE(CommandList);
		~CommandList() { ZE_ASSERT(commands == nullptr && allocator == nullptr, "Command list not freed before deletion!"); }

		void Free(GFX::Device& dev) noexcept { Free(); }

		void Open(GFX::Device& dev);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso);

		void Close(GFX::Device& dev);
		void Reset(GFX::Device& dev);

		void Draw(GFX::Device& dev, U32 vertexCount) const noexcept(!_ZE_DEBUG_GFX_API);
		void DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(!_ZE_DEBUG_GFX_API);
		void DrawFullscreen(GFX::Device& dev) const noexcept(!_ZE_DEBUG_GFX_API);
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(!_ZE_DEBUG_GFX_API);

#if _ZE_GFX_MARKERS
		void TagBegin(GFX::Device& dev, std::string_view tag, Pixel color) const noexcept;
		void TagEnd(GFX::Device& dev) const noexcept;
#endif

		// Gfx API Internal

		IGraphicsCommandList* GetList() noexcept { return commands.Get(); }
		void Open(Device& dev) { Open(dev, nullptr); }
		void Free() noexcept { commands = nullptr; allocator = nullptr; }

		void Init(Device& dev, QueueType type);
		void Close(Device& dev);
		void Reset(Device& dev);
	};
}