#pragma once
#include "GFX/API/DX/Shader.h"
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
	};
}