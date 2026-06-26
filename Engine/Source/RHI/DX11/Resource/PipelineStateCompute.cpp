#include "RHI/DX11/Resource/PipelineStateCompute.h"

namespace ZE::RHI::DX11::Resource
{
	Expected<PipelineStateCompute> PipelineStateCompute::Create(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Binding::Schema& binding) noexcept
	{
		PipelineStateCompute state;
		ZE_DX_RET_FAILED_EXPECT(dev.Get().dx11.GetDevice()->CreateComputeShader(shader.Get().dx11.GetBytecode()->GetBufferPointer(),
			shader.Get().dx11.GetBytecode()->GetBufferSize(), nullptr, &state.computeShader));
		ZE_DX_SET_ID(state.computeShader, *shader.Get().dx11.GetName());
		return state;
	}
}