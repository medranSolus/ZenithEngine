#include "GFX/API/DX12/Resource/PipelineStateCompute.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12::Resource
{
	PipelineStateCompute::PipelineStateCompute(GFX::Device& dev, const std::wstring& nameCS)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx12);

		DX::ComPtr<ID3DBlob> bytecode;
		ZE_GFX_THROW_FAILED(D3DReadFileToBlob((L"Shaders/" + nameCS + L".cso").c_str(), &bytecode));

		D3D12_COMPUTE_PIPELINE_STATE_DESC desc;
		desc.pRootSignature = nullptr; // TODO: Change later
		desc.CS.pShaderBytecode = bytecode->GetBufferPointer();
		desc.CS.BytecodeLength = bytecode->GetBufferSize();
		desc.NodeMask = 0;
		desc.CachedPSO.pCachedBlob = nullptr;
		desc.CachedPSO.CachedBlobSizeInBytes = 0;
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		ZE_GFX_THROW_FAILED(dev.Get().dx12.GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&state)));
		ZE_GFX_SET_ID(state, "CS");
	}
}