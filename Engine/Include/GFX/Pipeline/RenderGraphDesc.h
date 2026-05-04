#pragma once
#include "FrameBufferDesc.h"
#include "PassDesc.h"
#include "RenderNode.h"

namespace ZE::GFX::Pipeline
{
	// User-filled description of whole render graph with render passes and framebuffer resources
	struct RenderGraphDesc
	{
		FrameBufferFlags ResourceOptions = Base(FrameBufferFlag::None);
		std::vector<std::pair<std::string, FrameResourceDesc>> Resources;
		std::vector<RenderNode> StartupPasses;
		std::vector<RenderNode> RenderPasses;
		std::vector<Resource::SamplerDesc> Samplers;
		Binding::Range SettingsRange = {};
		Binding::Range DynamicDataRange = {};
		RendererSettingsData SettingsData = {};
		Ptr<PassExecuteData> PassCustomData;
		PassDebugUICallback PassCustomDataDebugUICallback = nullptr;

		PixelFormat GetFormat(std::string_view name) const noexcept;
		void InitBuffers() noexcept;
		void AddResource(std::string_view name, FrameResourceDesc&& desc) noexcept;
	};
}