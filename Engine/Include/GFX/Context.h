#pragma once
#include "API/DX11/Context.h"
#include "API/DX12/Context.h"
#include "CommandList.h"

namespace ZE::GFX
{
	// Sending commands to GPU
	class Context final
	{
		ZE_API_BACKEND(Context) backend;

	public:
		Context() = default;
		constexpr Context(Device& dev) { backend.Init(dev); }
		Context(Context&&) = default;
		Context(const Context&) = delete;
		Context& operator=(Context&&) = default;
		Context& operator=(const Context&) = delete;
		~Context() = default;

		constexpr void Init(Device& dev) { backend.Init(dev); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev) { backend.Switch(nextApi, dev); }
		constexpr ZE_API_BACKEND(Context)& Get() noexcept { return backend; }

#ifdef _ZE_MODE_DEBUG
		constexpr void TagBegin(const wchar_t* tag) const noexcept { ZE_API_BACKEND_CALL(backend, TagBegin, tag); }
		constexpr void TagEnd() const noexcept { ZE_API_BACKEND_CALL(backend, TagEnd); }
#endif
		constexpr void DrawIndexed(Device& dev, U32 count) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, DrawIndexed, dev, count); }
		constexpr void Compute(Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, Compute, dev, groupX, groupY, groupZ); }

		constexpr void Execute(Device& dev, CommandList& cl) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, Execute, dev, cl); }
		constexpr void Execute(Device& dev, CommandList* cl, U32 count) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, Execute, dev, cl, count); }
		constexpr void Execute(Device& dev, Context& ctx) const { ZE_API_BACKEND_CALL(backend, Execute, dev, ctx); }
		constexpr void Execute(Device& dev, Context* ctx, U32 count) const { ZE_API_BACKEND_CALL(backend, Execute, dev, ctx, count); }

		constexpr void CreateList(Device& dev, CommandList& cl) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, CreateList, dev, cl); }
		Context CreateDeffered(Device& dev) const { Context ctx; ZE_API_BACKEND_CALL(backend, CreateDeffered, dev, ctx); return ctx; }
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