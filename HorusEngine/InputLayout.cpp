#include "InputLayout.h"
#include "Codex.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	InputLayout::InputLayout(Graphics& gfx, std::shared_ptr<BasicType::VertexLayout> vertexLayout, ID3DBlob* vertexShaderBytecode)
		: vertexLayout(vertexLayout)
	{
		GFX_ENABLE_ALL(gfx);
		std::vector<D3D11_INPUT_ELEMENT_DESC> desc = vertexLayout->GetDXLayout();
		GFX_THROW_FAILED(GetDevice(gfx)->CreateInputLayout(desc.data(), static_cast<UINT>(desc.size()),
			vertexShaderBytecode->GetBufferPointer(), vertexShaderBytecode->GetBufferSize(), &inputLayout));
	}
}