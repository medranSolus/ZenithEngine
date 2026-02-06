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
		ZE_CLASS_MOVE(Shader);
		~Shader() = default;

		static Expected<Shader> Create(Device& dev, std::string_view name) { ZE_RHI_BACKEND_CREATE(Resource::Shader, dev, name); }
		ZE_RHI_BACKEND_GET(Resource::Shader);

		// Main Gfx API

#if _ZE_DEBUG_GFX_NAMES
		const std::string& GetName() const noexcept { const std::string* name = nullptr; ZE_RHI_BACKEND_CALL_RET_VAR(name, GetName); return name ? *name : ""; }
#endif
	};
}