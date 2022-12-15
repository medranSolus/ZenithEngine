#include "GFX/API/VK/CommandList.h"
#include "GFX/API/VK/VulkanException.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/PipelineStateGfx.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::VK
{
	void CommandList::Open(Device& dev, VkPipeline state, VkPipelineBindPoint bindPoint)
	{
		ZE_VK_ENABLE();

		VkCommandBufferInheritanceInfo inheritanceInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, nullptr };
		inheritanceInfo.renderPass = VK_NULL_HANDLE;
		inheritanceInfo.subpass = 0;
		inheritanceInfo.framebuffer = VK_NULL_HANDLE;
		inheritanceInfo.occlusionQueryEnable = VK_FALSE;
		inheritanceInfo.queryFlags = 0;
		inheritanceInfo.pipelineStatistics = 0;

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = &inheritanceInfo; // Only for secondary buffers
		ZE_VK_THROW_NOSUCC(vkBeginCommandBuffer(commands, &beginInfo));

		if (state)
			vkCmdBindPipeline(commands, bindPoint, state);
	}

	CommandList::CommandList(GFX::Device& dev, CommandType type)
	{
		Init(dev.Get().vk, type);
	}

	void CommandList::Open(GFX::Device& dev)
	{
		Open(dev.Get().vk, VK_NULL_HANDLE, VK_PIPELINE_BIND_POINT_GRAPHICS);
	}

	void CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso)
	{
		Open(dev.Get().vk, pso.Get().vk.GetState(), VK_PIPELINE_BIND_POINT_COMPUTE);
	}

	void CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso)
	{
		Open(dev.Get().vk, pso.Get().vk.GetState(), VK_PIPELINE_BIND_POINT_GRAPHICS);
		//commands->IASetPrimitiveTopology(pso.Get().dx12.GetTopology());
	}

	void CommandList::Reset(GFX::Device& dev)
	{
		Reset(dev.Get().vk);
	}

	void CommandList::Draw(GFX::Device& dev, U32 vertexCount) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		vkCmdDraw(commands, vertexCount, 1, 0, 0);
	}

	void CommandList::DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		vkCmdDrawIndexed(commands, indexCount, 1, 0, 0, 0);
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

	void CommandList::Free(GFX::Device& dev) noexcept
	{
		if (commands)
		{
			vkFreeCommandBuffers(dev.Get().vk.GetDevice(), pool, 1, &commands);
			commands = VK_NULL_HANDLE;
		}
		if (pool)
		{
			vkDestroyCommandPool(dev.Get().vk.GetDevice(), pool, nullptr);
			pool = VK_NULL_HANDLE;
		}
	}

	void CommandList::Init(Device& dev, CommandType type)
	{
		ZE_VK_ENABLE_ID();

		VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr };
		poolInfo.flags = 0;
		switch (type)
		{
		default:
		case ZE::GFX::CommandType::Bundle:
		case ZE::GFX::CommandType::All:
		{
			poolInfo.queueFamilyIndex = dev.GetGfxQueueIndex();
			break;
		}
		case ZE::GFX::CommandType::Compute:
		{
			poolInfo.queueFamilyIndex = dev.GetComputeQueueIndex();
			break;
		}
		case ZE::GFX::CommandType::Copy:
		{
			poolInfo.queueFamilyIndex = dev.GetCopyQueueIndex();
			break;
		}
		}
		ZE_VK_THROW_NOSUCC(vkCreateCommandPool(dev.GetDevice(), &poolInfo, nullptr, &pool));

		VkCommandBufferAllocateInfo commandsInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr };
		commandsInfo.commandPool = pool;
		commandsInfo.level = type == CommandType::Bundle ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandsInfo.commandBufferCount = 1;
		ZE_VK_THROW_NOSUCC(vkAllocateCommandBuffers(dev.GetDevice(), &commandsInfo, &commands));

#if _ZE_DEBUG_GFX_API
		switch (type)
		{
		default:
		case ZE::GFX::CommandType::All:
		{
			ZE_VK_SET_ID(dev.GetDevice(), pool, VK_OBJECT_TYPE_COMMAND_POOL, "direct_allocator");
			ZE_VK_SET_ID(dev.GetDevice(), commands, VK_OBJECT_TYPE_COMMAND_BUFFER, "direct_command");
			break;
		}
		case ZE::GFX::CommandType::Bundle:
		{
			ZE_VK_SET_ID(dev.GetDevice(), pool, VK_OBJECT_TYPE_COMMAND_POOL, "bundle_allocator");
			ZE_VK_SET_ID(dev.GetDevice(), commands, VK_OBJECT_TYPE_COMMAND_BUFFER, "bundle_command");
			break;
		}
		case ZE::GFX::CommandType::Compute:
		{
			ZE_VK_SET_ID(dev.GetDevice(), pool, VK_OBJECT_TYPE_COMMAND_POOL, "compute_allocator");
			ZE_VK_SET_ID(dev.GetDevice(), commands, VK_OBJECT_TYPE_COMMAND_BUFFER, "compute_command");
			break;
		}
		case ZE::GFX::CommandType::Copy:
		{
			ZE_VK_SET_ID(dev.GetDevice(), pool, VK_OBJECT_TYPE_COMMAND_POOL, "copy_allocator");
			ZE_VK_SET_ID(dev.GetDevice(), commands, VK_OBJECT_TYPE_COMMAND_BUFFER, "copy_command");
			break;
		}
		}
#endif
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
}