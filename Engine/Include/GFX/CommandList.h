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
		constexpr CommandList() { backend.Init(); }
		CommandList(CommandList&&) = default;
		CommandList(const CommandList&) = delete;
		CommandList& operator=(CommandList&&) = default;
		CommandList& operator=(const CommandList&) = delete;
		~CommandList() = default;

		constexpr void SwitchApi(GfxApiType nextApi) { backend.Switch(nextApi); }
		constexpr ZE_API_BACKEND(CommandList)& Get() noexcept { return backend; }
	};
}