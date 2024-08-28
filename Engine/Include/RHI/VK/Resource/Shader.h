#pragma once
#include "RHI/VK/VulkanExtensions.h"

namespace ZE::GFX
{
	class Device;
}
namespace ZE::RHI::VK::Resource
{
	class Shader final
	{
		VkShaderModule shader = VK_NULL_HANDLE;
#if _ZE_DEBUG_GFX_NAMES
		std::string shaderName = "";
#endif

	public:
		Shader() = default;
		Shader(GFX::Device& dev, std::string_view name);
		Shader(Shader&& shdr) noexcept;
		Shader& operator=(Shader&& shdr) noexcept;
		ZE_CLASS_NO_COPY(Shader);
		~Shader() { ZE_ASSERT_FREED(shader == VK_NULL_HANDLE); }

#if _ZE_DEBUG_GFX_NAMES
		constexpr const std::string* GetName() const noexcept { return &shaderName; }
#endif
		void Free(GFX::Device& dev) noexcept;

		// Gfx API Internal

		constexpr VkShaderModule GetModule() const noexcept { return shader; }
	};
}