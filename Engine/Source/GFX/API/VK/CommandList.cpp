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

	void CommandList::Draw(GFX::Device& dev, U32 vertexCount) const noexcept(ZE_NO_DEBUG)
	{
	}

	void CommandList::DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(ZE_NO_DEBUG)
	{
	}

	void CommandList::DrawFullscreen(GFX::Device& dev) const noexcept(ZE_NO_DEBUG)
	{
	}

	void CommandList::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG)
	{
	}
}