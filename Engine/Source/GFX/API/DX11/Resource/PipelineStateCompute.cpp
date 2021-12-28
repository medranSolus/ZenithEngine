#include "GFX/API/DX11/Resource/PipelineStateCompute.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11::Resource
{
	PipelineStateCompute::PipelineStateCompute(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Material::Schema& binding)
	{
		ZE_GFX_ENABLE_ID(dev.Get().dx11);

		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateComputeShader(shader.Get().dx11.GetBytecode()->GetBufferPointer(),
			shader.Get().dx11.GetBytecode()->GetBufferSize(), nullptr, &computeShader));
		ZE_GFX_SET_ID(computeShader, shader.Get().dx11.GetName());
	}
}