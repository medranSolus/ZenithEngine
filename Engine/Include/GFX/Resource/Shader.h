#pragma once
#include "GFX/API/DX/Shader.h"
#include "GFX/API/VK/Resource/Shader.h"
#include "GFX/API/Backend.h"

namespace ZE::GFX::Resource
{
	// Data of single shader to load by pipeline
	class Shader
	{
		ZE_API_BACKEND(Resource::Shader);

	public:
		constexpr Shader(const std::wstring& name) { ZE_API_BACKEND_VAR.Init(name); }
		ZE_CLASS_MOVE(Shader);
		~Shader() = default;

		ZE_API_BACKEND_GET(Resource::Shader);

		// Main Gfx API

#if _ZE_DEBUG_GFX_NAMES
		const std::string& GetName() const noexcept { ZE_API_BACKEND_CALL(GetName); }
#endif
	};
}