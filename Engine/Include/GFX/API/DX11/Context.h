#pragma once
#include "GFX/CommandList.h"
#include "GFX/Device.h"
#include "D3D11.h"

namespace ZE::GFX
{
	class Context;
}
namespace ZE::GFX::API::DX11
{
	class Context final
	{
		DX::ComPtr<ID3D11DeviceContext4> context;
#ifdef _ZE_MODE_DEBUG
		DX::ComPtr<ID3DUserDefinedAnnotation> tagManager;
#endif

		Context() = default;

	public:
		Context(GFX::Device& dev);
		Context(Context&&) = default;
		Context(const Context&) = delete;
		Context& operator=(Context&&) = default;
		Context& operator=(const Context&) = delete;
		~Context() = default;

		ID3D11DeviceContext4* GetContext() const noexcept { return context.Get(); }
#ifdef _ZE_MODE_DEBUG
		void TagBegin(const wchar_t* tag) const noexcept { tagManager->BeginEvent(tag); }
		void TagEnd() const noexcept { tagManager->EndEvent(); }
#endif

		void DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG);
		// For best performance each thread group should be multiple of 32 (ideally as many as 2*processors on GPU)
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG);

		void Execute(GFX::Device& dev, GFX::CommandList& cl) const noexcept(ZE_NO_DEBUG);
		void Execute(GFX::Device& dev, GFX::CommandList* cl, U32 count) const noexcept(ZE_NO_DEBUG);
		void Execute(GFX::Device& dev, GFX::Context& ctx) const;
		void Execute(GFX::Device& dev, GFX::Context* ctx, U32 count) const;

		void CreateList(GFX::Device& dev, GFX::CommandList& cl) const;
		void CreateDeffered(GFX::Device& dev, GFX::Context& ctx) const;
	};
}