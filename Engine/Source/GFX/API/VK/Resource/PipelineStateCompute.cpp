#include "GFX/API/VK/Resource/PipelineStateCompute.h"
#include "GFX/API/VK/VulkanException.h"

namespace ZE::GFX::API::VK::Resource
{
	PipelineStateCompute::PipelineStateCompute(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Binding::Schema& binding)
	{
		ZE_VK_ENABLE_ID();

		VkComputePipelineCreateInfo pipelineInfo;
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr;
		pipelineInfo.flags = 0;

		pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipelineInfo.stage.pNext = nullptr;
		pipelineInfo.stage.flags = dev.Get().vk.IsExtensionSupported(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME) ? VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT : 0;
		pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		pipelineInfo.stage.module = shader.Get().vk.GetModule();
		pipelineInfo.stage.pName = "main";
		pipelineInfo.stage.pSpecializationInfo = nullptr;

		pipelineInfo.layout = binding.Get().vk.GetLayout();
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = 0;
		ZE_VK_THROW_NOSUCC(vkCreateComputePipelines(dev.Get().vk.GetDevice(), 0, 1, &pipelineInfo, nullptr, &state));
		ZE_VK_SET_ID(dev.Get().vk.GetDevice(), state, VK_OBJECT_TYPE_PIPELINE, shader.Get().vk.GetName());
	}

	void PipelineStateCompute::Free(GFX::Device& dev) noexcept
	{
		if (state)
		{
			vkDestroyPipeline(dev.Get().vk.GetDevice(), state, nullptr);
			state = VK_NULL_HANDLE;
		}
	}
}