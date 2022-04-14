#pragma once
#include "DXGI.h"
#include "GraphicsException.h"

namespace ZE::GFX::API::DX
{
	template<bool IsDX12>
	class Shader final
	{
		ComPtr<ID3DBlob> bytecode;
#if _ZE_MODE_DEBUG
		std::string shaderName = "";
#endif

	public:
		Shader() = default;
		Shader(const std::wstring& name);
		ZE_CLASS_MOVE(Shader);
		~Shader() = default;

		// Gfx API Internal

#if _ZE_MODE_DEBUG
		constexpr const std::string& GetName() const noexcept { return shaderName; }
#endif
		ID3DBlob* GetBytecode() const noexcept { return bytecode.Get(); }
	};

#pragma region Functions
	template<bool IsDX12>
	Shader<IsDX12>::Shader(const std::wstring& name)
	{
		ZE_WIN_ENABLE_EXCEPT();
#if _ZE_MODE_DEBUG
		shaderName = Utils::ToAscii(name);
#endif
		// TODO: Seperate shader compilation for every target API and place it in seperate subdirectory
		if constexpr (IsDX12)
		{
			ZE_WIN_THROW_FAILED(D3DReadFileToBlob((L"Shaders/DX12/" + name + L".cso").c_str(), &bytecode));
		}
		else
		{
			ZE_WIN_THROW_FAILED(D3DReadFileToBlob((L"Shaders/DX11/" + name + L".cso").c_str(), &bytecode));
		}
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