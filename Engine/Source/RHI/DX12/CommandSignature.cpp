#include "RHI/DX12/CommandSignature.h"

namespace ZE::RHI::DX12
{
	Expected<CommandSignature> CommandSignature::Create(GFX::Device& dev, GFX::IndirectCommandType type) noexcept
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument = {};
		D3D12_COMMAND_SIGNATURE_DESC desc = {};
		desc.pArgumentDescs = &argument;
		desc.NumArgumentDescs = 1;

		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::IndirectCommandType::Dispatch:
		{
			argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
			desc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
			break;
		}
		}

		CommandSignature cmd = {};
		ZE_DX_RET_FAILED_EXPECT(dev.Get().dx12.GetDevice()->CreateCommandSignature(&desc, nullptr, IID_PPV_ARGS(&cmd.signature)));
#if _ZE_DEBUG_GFX_NAMES
		switch (type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::IndirectCommandType::Dispatch:
		{
			ZE_DX_SET_ID(cmd.signature, "Indirect command signature - Dispatch");
			break;
		}
		}
#endif
		return cmd;
	}
}