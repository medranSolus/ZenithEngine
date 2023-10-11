#pragma once
#if _ZE_RHI_DX11 || _ZE_RHI_DX12
#	include "RHI/DX/Shader.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Resource/Shader.h"
#endif
#include "RHI/Backend.h"

namespace ZE::GFX::Resource
{
	// Data of single shader to load by pipeline
	class Shader
	{
		ZE_RHI_BACKEND(Resource::Shader);

	public:
		Shader() = default;
		constexpr Shader(GFX::Device& dev, const std::string& name) { Init(dev, name); }
		ZE_CLASS_MOVE(Shader);
		~Shader() = default;

		constexpr void Init(Device& dev, const std::string& name) { ZE_RHI_BACKEND_VAR.Init(dev, name); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const std::string& name) { ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, name); }
		ZE_RHI_BACKEND_GET(Resource::Shader);

		// Main Gfx API

		// Before destroying shader you have to call this function for proper memory freeing
		constexpr void Free(GFX::Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
#if _ZE_DEBUG_GFX_NAMES
		const std::string& GetName() const noexcept { ZE_RHI_BACKEND_CALL(GetName); }
#endif
	};
}