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
		Constant() = default;
		ZE_CLASS_MOVE(Constant);
		~Constant() = default;

		static constexpr Expected<Constant> Create(Device& dev, const T& value) noexcept { ZE_RHI_BACKEND_CREATE(Resource::Constant<T>, dev, value); }
		ZE_RHI_BACKEND_GET(Resource::Constant<T>);

		// Main Gfx API

		constexpr Status Set(GFX::Device& dev, const T& value) noexcept { ZE_RHI_BACKEND_CALL(Set, dev, value); }
		constexpr void Bind(CommandList& cl, Binding::Context& bindCtx) const noexcept { ZE_RHI_BACKEND_CALL(Bind, cl, bindCtx); }
	};
}