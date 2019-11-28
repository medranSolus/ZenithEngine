#pragma once
#include "IBindable.h"

namespace GFX::Resource
{
	class VertexShader : public IBindable
	{
	protected:
		Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;

	public:
		VertexShader(Graphics& gfx, const std::wstring & path);

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->VSSetShader(vertexShader.Get(), nullptr, 0U); }
		inline ID3DBlob * GetBytecode() const noexcept { return bytecode.Get(); }
	};
}
