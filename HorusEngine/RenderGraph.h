#pragma once
#include "QueuePass.h"

namespace GFX::Pipeline
{
	class RenderGraph
	{
	public:
		RenderGraph() = default;
		RenderGraph(const RenderGraph&) = default;
		RenderGraph& operator=(const RenderGraph&) = default;
		virtual ~RenderGraph() = default;

		RenderPass::Base::QueuePass& GetRenderQueue(const std::string& passName);
	};
}