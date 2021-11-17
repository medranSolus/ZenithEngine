#pragma once
#include "RenderGraph.h"

namespace ZE::GFX::Pipeline
{
	// Physically Based Renderer
	class RendererPBR : public RenderGraph
	{
	public:
		RendererPBR() = default;
		RendererPBR(RendererPBR&&) = delete;
		RendererPBR(const RendererPBR&) = delete;
		RendererPBR& operator=(RendererPBR&&) = delete;
		RendererPBR& operator=(const RendererPBR&) = delete;
		virtual ~RendererPBR() = default;

		void Init(Device& dev, CommandList& mainList, U32 width, U32 height, bool minimizePassDistances, U32 shadowMapSize);
	};
}