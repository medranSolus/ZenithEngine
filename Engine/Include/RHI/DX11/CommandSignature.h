#pragma once
#include "GFX/Device.h"
#include "GFX/IndirectCommandType.h"

namespace ZE::RHI::DX11
{
	class CommandSignature final
	{
	public:
		CommandSignature() = default;
		CommandSignature(GFX::Device& dev, GFX::IndirectCommandType type) {}
		ZE_CLASS_MOVE(CommandSignature);
		~CommandSignature() {}

		constexpr void Free(GFX::Device& dev) noexcept {}
	};
}