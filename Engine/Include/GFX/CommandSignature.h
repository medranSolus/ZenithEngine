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
		ZE_CLASS_MOVE(CommandSignature);
		~CommandSignature() = default;

		static Expected<CommandSignature> Create(Device& dev, IndirectCommandType type) noexcept { ZE_RHI_BACKEND_CREATE(CommandSignature, dev, type); }
		ZE_RHI_BACKEND_GET(CommandSignature);

		// Main Gfx API
	};
}