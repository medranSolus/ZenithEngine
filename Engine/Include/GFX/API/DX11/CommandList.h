#pragma once
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "GFX/CommandType.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11
{
	class CommandList final
	{
		DX::ComPtr<ID3D11DeviceContext4> context;
		DX::ComPtr<ID3D11CommandList> commands;
#ifdef _ZE_MODE_DEBUG
		DX::ComPtr<ID3DUserDefinedAnnotation> tagManager;
#endif

	public:
		CommandList(GFX::Device& dev);
		CommandList(GFX::Device& dev, CommandType type);
		CommandList(CommandList&&) = default;
		CommandList(const CommandList&) = delete;
		CommandList& operator=(CommandList&&) = default;
		CommandList& operator=(const CommandList&) = delete;
		~CommandList() = default;

		constexpr void Open(GFX::Device& dev) {}
		constexpr void FinishBarriers() noexcept {}

#ifdef _ZE_MODE_DEBUG
		void TagBegin(const wchar_t* tag) const noexcept { tagManager->BeginEvent(tag); }
		void TagEnd() const noexcept { tagManager->EndEvent(); }
#endif
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso) { SetState(pso); }
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso) { SetState(pso); }
		void SetState(GFX::Resource::PipelineStateCompute& pso) { pso.Get().dx11.Bind(context.Get()); }
		void SetState(GFX::Resource::PipelineStateGfx& pso) { pso.Get().dx11.Bind(context.Get()); }
		void Reset(GFX::Device& dev) { commands = nullptr; }

		void Close(GFX::Device& dev);
		void DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG);
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG);

		// Gfx API Internal

		ID3D11DeviceContext4* GetContext() const noexcept { return context.Get(); }
		ID3D11CommandList* GetList() const noexcept { return commands.Get(); }
	};
}