#pragma once

namespace ZE::GFX::API::VK::Resource
{
	class Shader final
	{
#if _ZE_MODE_DEBUG
		std::string shaderName = "";
#endif

	public:
		Shader() = default;
		Shader(const std::wstring& name);
		ZE_CLASS_MOVE(Shader);
		~Shader() = default;

#if _ZE_MODE_DEBUG
		constexpr const std::string& GetName() const noexcept { return shaderName; }
#endif
	};
}
