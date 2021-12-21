#pragma once
#include "DXGI.h"

namespace ZE::GFX::API::DX
{
	class Shader final
	{
		ComPtr<ID3DBlob> bytecode;

	public:
		Shader(const std::wstring& name);
		ZE_CLASS_MOVE(Shader);
		~Shader() = default;

		// Gfx API Internal

		ID3DBlob* GetBytecode() const noexcept { return bytecode.Get(); }
	};
}

namespace ZE::GFX::API::DX11::Resource
{
	typedef DX::Shader Shader;
}
namespace ZE::GFX::API::DX12::Resource
{
	typedef DX::Shader Shader;
}