#include "RHI/VK/CommandList.h"
#include "RHI/VK/VulkanException.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "GFX/Device.h"

namespace ZE::RHI::VK
{
	CommandList::CommandList(GFX::Device& dev, GFX::QueueType type)
	{
		Init(dev.Get().vk, type);
	}

	void CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso)
	{
		Open();
		vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, pso.Get().vk.GetState());
	}

	void CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso)
	{
		Open();
		vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pso.Get().vk.GetState());
		vkCmdSetPrimitiveTopologyEXT(commands, pso.Get().vk.GetTopology());
	}

	void CommandList::Reset(GFX::Device& dev)
	{
		Reset(dev.Get().vk);
	}

	void CommandList::DrawFullscreen(GFX::Device& dev) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		//commands->IASetVertexBuffers(0, 0, nullptr);
		//commands->IASetIndexBuffer(nullptr);
		vkCmdDraw(commands, 3, 1, 0, 0);
	}

	void CommandList::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		vkCmdDispatch(commands, groupX, groupY, groupZ);
	}

	void CommandList::Free(GFX::Device& dev) noexcept
	{
		Free(dev.Get().vk);
	}

#if _ZE_GFX_MARKERS
	void CommandList::TagBegin(GFX::Device& dev, std::string_view tag, Pixel color) const noexcept
	{
		VkDebugUtilsLabelEXT labelInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, nullptr };
		labelInfo.pLabelName = tag.data();
		*reinterpret_cast<ColorF4*>(labelInfo.color) = { color.Red, color.Green, color.Blue, color.Alpha };
		vkCmdBeginDebugUtilsLabelEXT(commands, &labelInfo);
	}

	void CommandList::TagEnd(GFX::Device& dev) const noexcept
	{
		vkCmdEndDebugUtilsLabelEXT(commands);
	}
#endif

	void CommandList::Init(Device& dev, GFX::QueueType commandType)
	{
		ZE_VK_ENABLE_ID();
		switch (commandType)
		{
		default:
		case ZE::GFX::QueueType::Main:
		{
			familyIndex = dev.GetGfxQueueIndex();
			break;
		}
		case ZE::GFX::QueueType::Compute:
		{
			familyIndex = dev.GetComputeQueueIndex();
			break;
		}
		case ZE::GFX::QueueType::Copy:
		{
			familyIndex = dev.GetCopyQueueIndex();
			break;
		}
		}

		VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr };
		poolInfo.flags = 0;
		poolInfo.queueFamilyIndex = familyIndex;
		ZE_VK_THROW_NOSUCC(vkCreateCommandPool(dev.GetDevice(), &poolInfo, nullptr, &pool));

		VkCommandBufferAllocateInfo commandsInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr };
		commandsInfo.commandPool = pool;
		commandsInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandsInfo.commandBufferCount = 1;
		ZE_VK_THROW_NOSUCC(vkAllocateCommandBuffers(dev.GetDevice(), &commandsInfo, &commands));

#if _ZE_DEBUG_GFX_NAMES
		switch (commandType)
		{
		default:
		case GFX::QueueType::Main:
		{
			ZE_VK_SET_ID(dev.GetDevice(), pool, VK_OBJECT_TYPE_COMMAND_POOL, "direct_allocator");
			ZE_VK_SET_ID(dev.GetDevice(), commands, VK_OBJECT_TYPE_COMMAND_BUFFER, "direct_command");
			break;
		}
		case GFX::QueueType::Compute:
		{
			ZE_VK_SET_ID(dev.GetDevice(), pool, VK_OBJECT_TYPE_COMMAND_POOL, "compute_allocator");
			ZE_VK_SET_ID(dev.GetDevice(), commands, VK_OBJECT_TYPE_COMMAND_BUFFER, "compute_command");
			break;
		}
		case GFX::QueueType::Copy:
		{
			ZE_VK_SET_ID(dev.GetDevice(), pool, VK_OBJECT_TYPE_COMMAND_POOL, "copy_allocator");
			ZE_VK_SET_ID(dev.GetDevice(), commands, VK_OBJECT_TYPE_COMMAND_BUFFER, "copy_command");
			break;
		}
		}
#endif
	}

	void CommandList::Open()
	{
		ZE_VK_ENABLE();

		VkDeviceGroupCommandBufferBeginInfo deviceGroupInfo = { VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO, nullptr };
		deviceGroupInfo.deviceMask = 1;

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, &deviceGroupInfo };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr; // Only for secondary buffers
		ZE_VK_THROW_NOSUCC(vkBeginCommandBuffer(commands, &beginInfo));

		VkDescriptorBufferBindingInfoEXT descBufferInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT, nullptr };
		descBufferInfo.address = 0;
		descBufferInfo.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT
			| VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT;
		//vkCmdBindDescriptorBuffersEXT(commands, 1, &descBufferInfo);
	}

	void CommandList::Close()
	{
		ZE_VK_ENABLE();
		ZE_VK_THROW_NOSUCC(vkEndCommandBuffer(commands));
	}

	void CommandList::Reset(Device& dev)
	{
		ZE_VK_ENABLE();
		ZE_VK_THROW_NOSUCC(vkResetCommandPool(dev.GetDevice(), pool, 0));
	}

	void CommandList::Free(Device& dev) noexcept
	{
		if (commands)
		{
			vkFreeCommandBuffers(dev.GetDevice(), pool, 1, &commands);
			commands = VK_NULL_HANDLE;
		}
		if (pool)
		{
			vkDestroyCommandPool(dev.GetDevice(), pool, nullptr);
			pool = VK_NULL_HANDLE;
		}
	}

	void CommandList::TransferOwnership(VkBufferMemoryBarrier2& barrier) noexcept
	{
		barrier.dstStageMask = barrier.srcStageMask;
		barrier.dstAccessMask = barrier.srcAccessMask;
		barrier.dstQueueFamilyIndex = familyIndex;
		barrier.offset = 0;
		barrier.size = VK_WHOLE_SIZE;

		VkDependencyInfo depInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO, nullptr };
		depInfo.dependencyFlags = 0;
		depInfo.memoryBarrierCount = 0;
		depInfo.pMemoryBarriers = nullptr;
		depInfo.bufferMemoryBarrierCount = 1;
		depInfo.pBufferMemoryBarriers = &barrier;
		depInfo.imageMemoryBarrierCount = 0;
		depInfo.pImageMemoryBarriers = nullptr;
		vkCmdPipelineBarrier2(commands, &depInfo);
	}
}