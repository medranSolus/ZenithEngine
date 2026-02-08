#pragma once
#include "GFX/Device.h"
#include "GFX/IndirectCommandType.h"

namespace ZE::RHI::DX12
{
	class CommandSignature final
	{
		DX::ComPtr<ICommandSignature> signature;

	public:
		CommandSignature() = default;
		ZE_CLASS_MOVE(CommandSignature);
		~CommandSignature() = default;

		static Expected<CommandSignature> Create(GFX::Device& dev, GFX::IndirectCommandType type) noexcept;

		// Gfx API Internal

		ICommandSignature* GetSignature() const noexcept { return signature.Get(); }
	};
}