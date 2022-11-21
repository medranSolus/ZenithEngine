#pragma once
#include "DXGI.h"
#include "DirectXException.h"

namespace ZE::GFX::API::DX
{
	template<bool IS_DX12>
	class Shader final
	{
		ComPtr<ID3DBlob> bytecode;
#if _ZE_DEBUG_GFX_NAMES
		std::string shaderName = "";
#endif

	public:
		Shader() = default;
		Shader(const std::wstring& name);
		ZE_CLASS_MOVE(Shader);
		~Shader() = default;

#if _ZE_DEBUG_GFX_NAMES
		constexpr const std::string& GetName() const noexcept { return shaderName; }
#endif

		// Gfx API Internal

		ID3DBlob* GetBytecode() const noexcept { return bytecode.Get(); }
	};

#pragma region Functions
	template<bool IS_DX12>
	Shader<IS_DX12>::Shader(const std::wstring& name)
	{
		ZE_WIN_ENABLE_EXCEPT();
#if _ZE_DEBUG_GFX_NAMES
		shaderName = Utils::ToAscii(name);
#endif
		ZE_WIN_THROW_FAILED(D3DReadFileToBlob(((IS_DX12 ? L"Shaders/DX12/" : L"Shaders/DX11/") + name + L".cso").c_str(), &bytecode));
	}
#pragma endregion
}

namespace ZE::GFX::API::DX11::Resource
{
	typedef DX::Shader<false> Shader;
}
namespace ZE::GFX::API::DX12::Resource
{
	typedef DX::Shader<true> Shader;
}