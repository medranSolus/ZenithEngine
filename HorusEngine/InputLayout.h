#pragma once
#include "IBindable.h"

namespace GFX::Resource
{
	class InputLayout : public IBindable
	{
	protected:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	public:
		InputLayout(Graphics & gfx, const std::vector<D3D11_INPUT_ELEMENT_DESC> & layoutDescs, ID3DBlob * vertexShaderBytecode);

		inline void Bind(Graphics & gfx) noexcept override { GetContext(gfx)->IASetInputLayout(inputLayout.Get()); }
	};
}
