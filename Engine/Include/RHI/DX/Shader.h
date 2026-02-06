#pragma once
#include "DXGI.h"

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
		ZE_CLASS_MOVE(Shader);
		~Shader() = default;

		static Expected<Shader> Create(GFX::Device& dev, std::string_view name) noexcept;

#if _ZE_DEBUG_GFX_NAMES
		constexpr const std::string* GetName() const noexcept { return &shaderName; }
#endif

		// Gfx API Internal

		ID3DBlob* GetBytecode() const noexcept { return bytecode.Get(); }
	};

#pragma region Functions
	template<bool IS_DX12>
	Expected<Shader<IS_DX12>> Shader<IS_DX12>::Create(GFX::Device& dev, std::string_view name) noexcept
	{
		Shader shader = {};
		if constexpr (IS_DX12)
		{
			ZE_DX_RET_FAILED_EXPECT(D3DReadFileToBlob((L"Shaders/DX12/" + Utils::ToUTF16(name) + L".dxil").c_str(), &shader.bytecode));
		}
		else
		{
			ZE_DX_RET_FAILED_EXPECT(D3DReadFileToBlob((L"Shaders/DX11/" + Utils::ToUTF16(name) + L".dxbc").c_str(), &shader.bytecode));
		}
#if _ZE_DEBUG_GFX_NAMES
		shader.shaderName = name;
#endif
		return shader;
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