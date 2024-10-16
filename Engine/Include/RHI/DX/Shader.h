#pragma once
#include "DXGI.h"
#include "DirectXException.h"

namespace ZE::GFX
{
	class Device;
}
namespace ZE::RHI::DX
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
		Shader(GFX::Device& dev, std::string_view name);
		ZE_CLASS_MOVE(Shader);
		~Shader() { ZE_ASSERT_FREED(bytecode == nullptr); }

		constexpr void Free(GFX::Device& dev) noexcept { bytecode = nullptr; }
#if _ZE_DEBUG_GFX_NAMES
		constexpr const std::string& GetName() const noexcept { return shaderName; }
#endif

		// Gfx API Internal

		ID3DBlob* GetBytecode() const noexcept { return bytecode.Get(); }
	};

#pragma region Functions
	template<bool IS_DX12>
	Shader<IS_DX12>::Shader(GFX::Device& dev, std::string_view name)
	{
		ZE_WIN_ENABLE_EXCEPT();
#if _ZE_DEBUG_GFX_NAMES
		shaderName = name;
#endif
		if constexpr (IS_DX12)
		{
			ZE_WIN_THROW_FAILED(D3DReadFileToBlob((L"Shaders/DX12/" + Utils::ToUTF16(name) + L".dxil").c_str(), &bytecode));
		}
		else
		{
			ZE_WIN_THROW_FAILED(D3DReadFileToBlob((L"Shaders/DX11/" + Utils::ToUTF16(name) + L".dxbc").c_str(), &bytecode));
		}
	}
#pragma endregion
}
namespace ZE::RHI::DX11::Resource
{
	typedef DX::Shader<false> Shader;
}
namespace ZE::RHI::DX12::Resource
{
	typedef DX::Shader<true> Shader;
}