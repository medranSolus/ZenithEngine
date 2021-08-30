#include "GFX/API/DX11/Resource/PipelineStateCompute.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11::Resource
{
	PipelineStateCompute::PipelineStateCompute(GFX::Device& dev, const std::wstring& nameCS)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx11);
		DX::ComPtr<ID3DBlob> bytecode;

		ZE_GFX_THROW_FAILED(D3DReadFileToBlob((L"Shaders/" + nameCS + L".cso").c_str(), &bytecode));
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateComputeShader(bytecode->GetBufferPointer(),
			bytecode->GetBufferSize(), nullptr, &shader));
		ZE_GFX_SET_ID(shader, "CS");
	}
}