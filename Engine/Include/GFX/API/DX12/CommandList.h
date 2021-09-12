#pragma once
#include "D3D12.h"

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
		DX::ComPtr<ID3D12GraphicsCommandList5> commands;
		DX::ComPtr<ID3D12CommandAllocator> allocator;
#ifdef _ZE_MODE_DEBUG
		DX::ComPtr<ID3DUserDefinedAnnotation> tagManager; // Maybe use PIX instead?
#endif

		void Open(Device& dev, ID3D12PipelineState* state);

	public:
		CommandList() = default;
		CommandList(GFX::Device& dev, CommandType type);
		CommandList(GFX::Device& dev);
		CommandList(CommandList&&) = default;
		CommandList(const CommandList&) = delete;
		CommandList& operator=(CommandList&&) = default;
		CommandList& operator=(const CommandList&) = delete;
		~CommandList() = default;

#ifdef _ZE_MODE_DEBUG
		void TagBegin(const wchar_t* tag) const noexcept { tagManager->BeginEvent(tag); }
		void TagEnd() const noexcept { tagManager->EndEvent(); }
#endif

		void Open(GFX::Device& dev);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso);
		void SetState(GFX::Resource::PipelineStateCompute& pso);
		void SetState(GFX::Resource::PipelineStateGfx& pso);

		void Close(GFX::Device& dev);
		void Reset(GFX::Device& dev);
		void DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG);
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG);

		// Gfx API Internal

		ID3D12GraphicsCommandList5* GetList() noexcept { return commands.Get(); }

		void Init(Device& dev, CommandType type);
		void Open(Device& dev);
		void Close(Device& dev);
	};
}