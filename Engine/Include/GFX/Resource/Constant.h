#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Resource/Constant.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Resource/Constant.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Resource/Constant.h"
#endif

namespace ZE::GFX::Resource
{
	// Constant passed directly to a shader, must be multiple of 4 bytes
	template<typename T>
	class Constant final
	{
		static_assert(sizeof(T) % 4 == 0, "Size of a constant must be a multiple of 4 bytes!");
		ZE_RHI_BACKEND(Resource::Constant<T>);

	public:
		constexpr Constant(Device& dev, const T& value) { ZE_RHI_BACKEND_VAR.Init(dev, value); }
		ZE_CLASS_MOVE(Constant);
		~Constant() = default;

		constexpr void SwitchApi(GfxApiType nextApi, Device& dev);
		ZE_RHI_BACKEND_GET(Resource::Constant<T>);

		// Main Gfx API

		constexpr void Set(GFX::Device& dev, const T& value) { ZE_RHI_BACKEND_CALL(Set, dev, value); }
		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx) const noexcept { ZE_RHI_BACKEND_CALL(Bind, cl, bindCtx); }
	};

#pragma region Functions
	template<typename T>
	constexpr void Constant<T>::SwitchApi(GfxApiType nextApi, Device& dev)
	{
		T data;
		ZE_RHI_BACKEND_CALL_RET(data, GetData, dev);
		ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, std::move(data));
	}
#pragma endregion
}