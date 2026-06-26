#pragma once
#include "GFX/Device.h"
#include "GFX/IndirectCommandType.h"

namespace ZE::RHI::DX11
{
	class CommandSignature final
	{
		GFX::IndirectCommandType type = GFX::IndirectCommandType::Dispatch;

	public:
		CommandSignature() = default;
		ZE_CLASS_MOVE(CommandSignature);
		~CommandSignature() = default;

		static Expected<CommandSignature> Create(GFX::Device& dev, GFX::IndirectCommandType type) noexcept { CommandSignature sig; sig.type = type; return sig; }

		// Gfx API Internal

		constexpr GFX::IndirectCommandType GetType() const noexcept { return type; }
	};
}