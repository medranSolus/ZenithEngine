#pragma once
#include "FrameBufferDesc.h"
#include "RenderNode.h"

namespace ZE::GFX::Pipeline
{
	// User-filled description of whole render graph with render passes and framebuffer resources
	// Also user provided callbacks should be here for various parts of render graph
	// (ex. handling of data) as well as pointers to custom data structures
	struct RenderGraphDesc
	{
		FrameBufferFlags ResourceOptions;
		std::vector<std::pair<std::string, FrameResourceDesc>> Resources;
		std::vector<RenderNode> RenderPasses;
		std::vector<Resource::SamplerDesc> Samplers;
		Binding::Range SettingsRange;
		Binding::Range DynamicDataRange;
		RendererSettingsData SettingsData;
		PtrVoid PassCustomData;
		// TODO: here comes all the custom callbacks and pointers

		PixelFormat GetFormat(std::string_view name) const noexcept;
		void InitBuffers() noexcept;
		void AddResource(std::string_view name, FrameResourceDesc&& desc) noexcept;
	};
}