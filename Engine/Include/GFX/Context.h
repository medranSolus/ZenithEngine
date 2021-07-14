#pragma once
#include "API/DX11/Context.h"
#include "CommandList.h"

namespace ZE::GFX
{
	// Sending commands to GPU
	class Context final
	{
		ZE_API_BACKEND(Context, DX11, DX11, DX11, DX11) backend;

	public:
		Context() = default;
		constexpr Context(Device& dev, bool main) { backend.Init(dev, main); }
		Context(Context&&) = delete;
		Context(const Context&) = delete;
		Context& operator=(Context&&) = delete;
		Context& operator=(const Context&) = delete;
		~Context() = default;

		constexpr void Init(Device& dev, bool main) { backend.Init(dev, main); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, bool main) { backend.Switch(nextApi, dev, main); }
		constexpr ZE_API_BACKEND(Context, DX11, DX11, DX11, DX11)& Get() noexcept { return backend; }

#ifdef _ZE_MODE_DEBUG
		void TagBegin(const wchar_t* tag) const noexcept { ZE_API_BACKEND_CALL(backend, TagBegin, tag); }
		void TagEnd() const noexcept { ZE_API_BACKEND_CALL(backend, TagEnd); }
#endif
		void DrawIndexed(Device& dev, U32 count) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, DrawIndexed, dev, count); }
		void Compute(Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, Compute, dev, groupX, groupY, groupZ); }
		void Execute(Device& dev, CommandList& cl) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, Execute, dev, cl); }
		void CreateList(Device& dev, CommandList& cl) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, CreateList, dev, cl); }
	};
}

#ifdef _ZE_MODE_DEBUG
// Begin debug tagged section
#define ZE_TAG_BEGIN(ctx, tag) ctx.TagBegin(tag)
// End last debug tagged section
#define ZE_TAG_END(ctx) ctx.TagEnd()
#else
// Begin debug tagged section
#define ZE_TAG_BEGIN(ctx, tag)
// End last debug tagged section
#define ZE_TAG_END(ctx)
#endif