#include "GFX/API/VK/CommandList.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::API::VK
{
	CommandList::CommandList(GFX::Device& dev)
	{
	}

	CommandList::CommandList(GFX::Device& dev, CommandType type)
	{
	}

	void CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso)
	{
	}

	void CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso)
	{
	}

	void CommandList::Close(GFX::Device& dev)
	{
	}

	void CommandList::Draw(GFX::Device& dev, U32 vertexCount) const noexcept(!_ZE_DEBUG_GFX_API)
	{
	}

	void CommandList::DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(!_ZE_DEBUG_GFX_API)
	{
	}

	void CommandList::DrawFullscreen(GFX::Device& dev) const noexcept(!_ZE_DEBUG_GFX_API)
	{
	}

	void CommandList::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(!_ZE_DEBUG_GFX_API)
	{
	}

#if _ZE_GFX_MARKERS
	void CommandList::TagBegin(GFX::Device& dev, const std::string_view tag, Pixel color) const noexcept
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
}