#include "InputLayout.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	InputLayout::InputLayout(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout, std::shared_ptr<VertexShader> shader)
		: vertexLayout(vertexLayout), shader(shader)
	{
		GFX_ENABLE_ALL(gfx);
		std::vector<D3D11_INPUT_ELEMENT_DESC> desc = vertexLayout->GetDXLayout();
		ID3DBlob* vertexShaderBytecode = shader->GetBytecode();
		GFX_THROW_FAILED(GetDevice(gfx)->CreateInputLayout(desc.data(), static_cast<UINT>(desc.size()),
			vertexShaderBytecode->GetBufferPointer(), vertexShaderBytecode->GetBufferSize(), &inputLayout));
	}
}