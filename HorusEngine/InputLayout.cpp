#include "InputLayout.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	InputLayout::InputLayout(Graphics & gfx, const std::vector<D3D11_INPUT_ELEMENT_DESC>& layoutDescs, ID3DBlob * vertexShaderBytecode)
	{
		GFX_ENABLE_ALL(gfx);
		GFX_THROW_FAILED(GetDevice(gfx)->CreateInputLayout(layoutDescs.data(), static_cast<UINT>(layoutDescs.size()), 
			vertexShaderBytecode->GetBufferPointer(), vertexShaderBytecode->GetBufferSize(), &inputLayout));
	}
}
