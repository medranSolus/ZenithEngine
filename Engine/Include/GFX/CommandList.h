#pragma once
#include "API/DX11/CommandList.h"
#include "API/DX12/CommandList.h"
#include "API/Backend.h"

namespace ZE::GFX
{
	// Storing commands for GPU
	class CommandList final
	{
		ZE_API_BACKEND(CommandList) backend;

	public:
		CommandList() = default;
		constexpr CommandList(Device& dev, CommandType type) { backend.Init(dev, type); }
		CommandList(CommandList&&) = default;
		CommandList(const CommandList&) = delete;
		CommandList& operator=(CommandList&&) = default;
		CommandList& operator=(const CommandList&) = delete;
		~CommandList() = default;

		constexpr void Init(Device& dev) { backend.Init(dev); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, CommandType type) { backend.Switch(nextApi, dev, type); }
		constexpr ZE_API_BACKEND(CommandList)& Get() noexcept { return backend; }

#ifdef _ZE_MODE_DEBUG
		constexpr void TagBegin(const wchar_t* tag) const noexcept { ZE_API_BACKEND_CALL(backend, TagBegin, tag); }
		constexpr void TagEnd() const noexcept { ZE_API_BACKEND_CALL(backend, TagEnd); }
#endif
		constexpr void Open(Device& dev, Resource::PipelineStateCompute& pso) { ZE_API_BACKEND_CALL(backend, Open, dev, pso); }
		constexpr void Open(Device& dev, Resource::PipelineStateGfx& pso) { ZE_API_BACKEND_CALL(backend, Open, dev, pso); }
		constexpr void Close(Device& dev) { ZE_API_BACKEND_CALL(backend, Close, dev); }
		constexpr void Reset(Device& dev) { ZE_API_BACKEND_CALL(backend, Reset, dev); }

		constexpr void DrawIndexed(Device& dev, U32 count) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, DrawIndexed, dev, count); }
		// For best performance each thread group should be multiple of 32 (ideally as many as 2*processors on GPU)
		constexpr void Compute(Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, Compute, dev, groupX, groupY, groupZ); }
	};
}