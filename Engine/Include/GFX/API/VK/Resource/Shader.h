#pragma once
#include "GFX/API/VK/VK.h"

namespace ZE::GFX
{
	class Device;

	namespace API::VK::Resource
	{
		class Shader final
		{
			VkShaderModule shader = VK_NULL_HANDLE;
#if _ZE_DEBUG_GFX_NAMES
			std::string shaderName = "";
#endif

		public:
			Shader() = default;
			Shader(GFX::Device& dev, const std::string& name);
			ZE_CLASS_MOVE(Shader);
			~Shader() { ZE_ASSERT(shader == VK_NULL_HANDLE, "Shader not freed before deletion!"); }

#if _ZE_DEBUG_GFX_NAMES
			constexpr const std::string& GetName() const noexcept { return shaderName; }
#endif
			void Free(GFX::Device& dev) noexcept;

			// Gfx API Internal

			constexpr VkShaderModule GetModule() const noexcept { return shader; }
		};
	}
}