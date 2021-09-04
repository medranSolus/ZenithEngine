#pragma once
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	class CommandList final
	{
		DX::ComPtr<ID3D12GraphicsCommandList5> commands;
		DX::ComPtr<ID3D12CommandAllocator> allocator;
#ifdef _ZE_MODE_DEBUG
		DX::ComPtr<ID3DUserDefinedAnnotation> tagManager; // Maybe use PIX instead?
#endif

		void Open(Device& dev, ID3D12PipelineState* state);

	public:
		CommandList(GFX::Device& dev);
		CommandList(GFX::Device& dev, CommandType type);
		CommandList(CommandList&&) = default;
		CommandList(const CommandList&) = delete;
		CommandList& operator=(CommandList&&) = default;
		CommandList& operator=(const CommandList&) = delete;
		~CommandList() = default;

#ifdef _ZE_MODE_DEBUG
		void TagBegin(const wchar_t* tag) const noexcept { tagManager->BeginEvent(tag); }
		void TagEnd() const noexcept { tagManager->EndEvent(); }
#endif
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso) { Open(dev.Get().dx12, pso.Get().dx12.GetState()); }
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso) { Open(dev.Get().dx12, pso.Get().dx12.GetState()); }

		void Close(GFX::Device& dev);
		void Reset(GFX::Device& dev);
		void DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG);
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG);

		// Gfx API Internal

		ID3D12GraphicsCommandList5* GetList() noexcept { return commands.Get(); }
	};
}