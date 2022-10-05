#include "GFX/API/DX11/Resource/PipelineStateCompute.h"

namespace ZE::GFX::API::DX11::Resource
{
	PipelineStateCompute::PipelineStateCompute(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Binding::Schema& binding)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx11);

		ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateComputeShader(shader.Get().dx11.GetBytecode()->GetBufferPointer(),
			shader.Get().dx11.GetBytecode()->GetBufferSize(), nullptr, &computeShader));
		ZE_DX_SET_ID(computeShader, shader.Get().dx11.GetName());
	}
}