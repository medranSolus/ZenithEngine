#include "RHI/VK/Pipeline/FrameBuffer.h"

namespace ZE::RHI::VK::Pipeline
{
	FrameBuffer::FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList,
		const GFX::Pipeline::FrameBufferDesc& desc)
	{
	}

	FrameBuffer::~FrameBuffer()
	{
	}

	void FrameBuffer::BeginRaster(GFX::CommandList& cl)
	{
		VkRenderingInfo renderInfo = { VK_STRUCTURE_TYPE_RENDERING_INFO, nullptr };
		renderInfo.flags = 0;
		renderInfo.renderArea.offset.x = 0;
		renderInfo.renderArea.offset.y = 0;
		renderInfo.renderArea.extent.width;
		renderInfo.renderArea.extent.height;
		renderInfo.layerCount;
		renderInfo.viewMask = 0;
		renderInfo.colorAttachmentCount;
		renderInfo.pColorAttachments;
		renderInfo.pDepthAttachment;
		renderInfo.pStencilAttachment;
		vkCmdBeginRendering(cl.Get().vk.GetBuffer(), &renderInfo);
	}

	void FrameBuffer::EndRaster(GFX::CommandList& cl)
	{
		vkCmdEndRendering(cl.Get().vk.GetBuffer());
	}
}