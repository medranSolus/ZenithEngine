#pragma once
#include "GFX/CommandList.h"
#include "GFX/Device.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	class Context final
	{
		DX::ComPtr<ID3D12GraphicsCommandList5> list;
#ifdef _ZE_MODE_DEBUG
		DX::ComPtr<ID3DUserDefinedAnnotation> tagManager; // Maybe use PIX instead?
#endif

	public:
		Context(GFX::Device& dev, bool main);
		Context(Context&&) = delete;
		Context(const Context&) = delete;
		Context& operator=(Context&&) = delete;
		Context& operator=(const Context&) = delete;
		~Context() = default;

		ID3D12GraphicsCommandList5* GetList() const noexcept { return list.Get(); }
#ifdef _ZE_MODE_DEBUG
		void TagBegin(const wchar_t* tag) const noexcept { tagManager->BeginEvent(tag); }
		void TagEnd() const noexcept { tagManager->EndEvent(); }
#endif

		void DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG);
		// For best performance each thread group should be multiple of 32 (ideally as many as 2*processors on GPU)
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG);
		void Execute(GFX::Device& dev, GFX::CommandList& cl) const noexcept(ZE_NO_DEBUG);
		void CreateList(GFX::Device& dev, GFX::CommandList& cl) const;
	};
}