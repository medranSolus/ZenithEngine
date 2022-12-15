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
		constexpr Shader(GFX::Device& dev, const std::wstring& name) { ZE_API_BACKEND_VAR.Init(dev, name); }
		ZE_CLASS_MOVE(Shader);
		~Shader() = default;

		ZE_API_BACKEND_GET(Resource::Shader);

		// Main Gfx API

		// Before destroying shader you have to call this function for proper memory freeing
		constexpr void Free(GFX::Device& dev) noexcept { ZE_API_BACKEND_CALL(Free, dev); }
#if _ZE_DEBUG_GFX_NAMES
		const std::string& GetName() const noexcept { ZE_API_BACKEND_CALL(GetName); }
#endif
	};
}