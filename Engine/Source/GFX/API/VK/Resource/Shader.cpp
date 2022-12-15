#include "GFX/API/VK/Resource/Shader.h"
#include "GFX/API/VK/VulkanException.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::VK::Resource
{
	Shader::Shader(GFX::Device& dev, const std::wstring& name)
	{
		ZE_VK_ENABLE_ID();
#if _ZE_DEBUG_GFX_NAMES
		shaderName = Utils::ToAscii(name);
#endif
		std::ifstream fin(L"Shaders/Vk/" + name + L".spv", std::ios::ate | std::ios::binary);
		if (!fin.good())
			throw ZE_IO_EXCEPT("Cannot load Vulkan shader: " + Utils::ToAscii(name));

		std::vector<char> bytecode(fin.tellg());
		fin.seekg(0);
		fin.read(bytecode.data(), bytecode.size());
		fin.close();

		VkShaderModuleCreateInfo shaderInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr };
		shaderInfo.flags = 0;
		shaderInfo.codeSize = bytecode.size();
		shaderInfo.pCode = reinterpret_cast<U32*>(bytecode.data());
		ZE_VK_THROW_NOSUCC(vkCreateShaderModule(dev.Get().vk.GetDevice(), &shaderInfo, nullptr, &shader));
		ZE_VK_SET_ID(dev.Get().vk.GetDevice(), shader, VK_OBJECT_TYPE_SHADER_MODULE, Utils::ToAscii(name));
	}

	void Shader::Free(GFX::Device& dev) noexcept
	{
		if (shader)
		{
			vkDestroyShaderModule(dev.Get().vk.GetDevice(), shader, nullptr);
			shader = VK_NULL_HANDLE;
		}
	}
}