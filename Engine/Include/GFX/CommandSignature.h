#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/CommandSignature.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/CommandSignature.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/CommandSignature.h"
#endif

namespace ZE::GFX
{
	// Signature of commands used for indirect drawing
	class CommandSignature final
	{
		ZE_RHI_BACKEND(CommandSignature);

	public:
		CommandSignature() = default;
		constexpr CommandSignature(Device& dev, IndirectCommandType type) { Init(dev, type); }
		ZE_CLASS_MOVE(CommandSignature);
		~CommandSignature() = default;

		constexpr void Init(Device& dev, IndirectCommandType type) { ZE_RHI_BACKEND_VAR.Init(dev, type); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, IndirectCommandType type) { ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, type); }
		ZE_RHI_BACKEND_GET(CommandSignature);

		// Main Gfx API

		// Before destroying buffer you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};
}