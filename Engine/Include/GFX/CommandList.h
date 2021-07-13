#pragma once
#include "API/DX11/CommandList.h"
#include "API/Backend.h"

namespace ZE::GFX
{
	// Storing commands for GPU
	class CommandList final
	{
		ZE_API_BACKEND(CommandList, DX11, DX11, DX11, DX11) backend;

	public:
		constexpr CommandList() { backend.Init(); }
		CommandList(CommandList&&) = delete;
		CommandList(const CommandList&) = delete;
		CommandList& operator=(CommandList&&) = delete;
		CommandList& operator=(const CommandList&) = delete;
		~CommandList() = default;

		constexpr void SwitchApi(GfxApiType nextApi) { backend.Switch(nextApi); }
		constexpr ZE_API_BACKEND(CommandList, DX11, DX11, DX11, DX11)& Get() noexcept { return backend; }
	};
}