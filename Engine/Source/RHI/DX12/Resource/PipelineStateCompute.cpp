#include "RHI/DX12/Resource/PipelineStateCompute.h"

namespace ZE::RHI::DX12::Resource
{
	PipelineStateCompute::PipelineStateCompute(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Binding::Schema& binding)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx12);

		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = binding.Get().dx12.GetSignature();
		desc.CS.pShaderBytecode = shader.Get().dx12.GetBytecode()->GetBufferPointer();
		desc.CS.BytecodeLength = shader.Get().dx12.GetBytecode()->GetBufferSize();
		desc.NodeMask = 0;
		desc.CachedPSO.pCachedBlob = nullptr;
		desc.CachedPSO.CachedBlobSizeInBytes = 0;
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		ZE_DX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&state)));
		ZE_DX_SET_ID(state, shader.Get().dx12.GetName());
	}
}