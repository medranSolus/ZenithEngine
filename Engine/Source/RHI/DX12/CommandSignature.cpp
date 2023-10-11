#include "RHI/DX12/CommandSignature.h"

namespace ZE::RHI::DX12
{
	CommandSignature::CommandSignature(GFX::Device& dev, GFX::IndirectCommandType type)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx12);

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
		ZE_DX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateCommandSignature(&desc, nullptr, IID_PPV_ARGS(&signature)));
		ZE_DX_SET_ID(signature, "Indirect command signature");
	}
}