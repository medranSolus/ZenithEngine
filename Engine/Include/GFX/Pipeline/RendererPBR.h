#pragma once
#include "RenderGraph.h"

namespace ZE::GFX::Pipeline
{
	class RendererPBR : public RenderGraph
	{
	public:
		RendererPBR() = default;
		RendererPBR(RendererPBR&&) = delete;
		RendererPBR(const RendererPBR&) = delete;
		RendererPBR& operator=(RendererPBR&&) = delete;
		RendererPBR& operator=(const RendererPBR&) = delete;
		virtual ~RendererPBR() = default;

		void Init(U32 width, U32 height, U32 shadowMapSize = 1024);
	};
}