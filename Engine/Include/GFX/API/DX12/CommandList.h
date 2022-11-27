#pragma once
#include "D3D12.h"
#include "Table.h"
#include "WarningGuardOn.h"
#include "WinPixEventRuntime/pix3.h"
#include "WarningGuardOff.h"

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
		CommandList(GFX::Device& dev, CommandType type);
		CommandList(GFX::Device& dev);
		ZE_CLASS_MOVE(CommandList);
		~CommandList() = default;

#if _ZE_GFX_MARKERS
		void TagBegin(GFX::Device& dev, const wchar_t* tag, Pixel color) const noexcept;
		void TagEnd(GFX::Device& dev) const noexcept;
#endif

		void Open(GFX::Device& dev);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso);

		void Close(GFX::Device& dev);
		void Reset(GFX::Device& dev);
		void Draw(GFX::Device& dev, U32 vertexCount) const noexcept(!_ZE_DEBUG_GFX_API);
		void DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(!_ZE_DEBUG_GFX_API);
		void DrawFullscreen(GFX::Device& dev) const noexcept(!_ZE_DEBUG_GFX_API);
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(!_ZE_DEBUG_GFX_API);

		// Gfx API Internal

		IGraphicsCommandList* GetList() noexcept { return commands.Get(); }

		void Init(Device& dev, CommandType type);
		void Open(Device& dev);
		void Close(Device& dev);
		void Reset(Device& dev);
	};
}