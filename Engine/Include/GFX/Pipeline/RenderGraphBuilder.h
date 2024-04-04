#pragma once
#include "RenderGraphDesc.h"

namespace ZE::GFX::Pipeline
{
	// Builder class that should hold and manage intermediate form of render graph description with most of things precomputed
	// but allowing for updating whole graph in case of hard update. With soft updates it should allow for redirection of resources
	// and give sufficient info about it's transitions.
	//
	// Maybe should merge together with RenderGraph instead? Depends what relations between resources of both classes will be
	class RenderGraphBuilder final
	{
	public:
		RenderGraphBuilder() = default;
		ZE_CLASS_MOVE(RenderGraphBuilder);
		~RenderGraphBuilder() = default;
	};
}