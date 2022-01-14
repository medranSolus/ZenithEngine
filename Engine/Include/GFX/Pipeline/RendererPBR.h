#pragma once
#include "RenderGraph.h"

namespace ZE::GFX::Pipeline
{
	// Physically Based Renderer
	class RendererPBR : public RenderGraph
	{
	public:
		RendererPBR() = default;
		ZE_CLASS_DELETE(RendererPBR);
		virtual ~RendererPBR() = default;

		void Init(Device& dev, CommandList& mainList, Resource::TextureLibrary& texLib,
			U32 width, U32 height, bool minimizePassDistances, U32 shadowMapSize);
	};
}