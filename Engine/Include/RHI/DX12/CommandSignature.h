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
		CommandSignature(GFX::Device& dev, GFX::IndirectCommandType type);
		ZE_CLASS_MOVE(CommandSignature);
		~CommandSignature() { ZE_ASSERT_FREED(signature == nullptr); }

		void Free(GFX::Device& dev) noexcept { signature = nullptr; }

		// Gfx API Internal

		ICommandSignature* GetSignature() const noexcept { return signature.Get(); }
	};
}