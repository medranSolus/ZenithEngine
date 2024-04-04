#pragma once
#include "Data/AssetsStreamer.h"
#include "GFX/Binding/Library.h"
#include "GFX/Resource/CBuffer.h"
#include "GFX/Resource/DynamicCBuffer.h"
#include "GFX/Resource/Shader.h"
#include "FrameBuffer.h"
#include "RendererData.h"

namespace ZE::GFX::Pipeline
{
	// Data passed to render pass during building step
	struct RendererPassBuildData
	{
		// Allows creation of materials and data bindings. Always save only index as Schema address may change during setup
		Binding::Library& BindingLib;
		// Allows creation of resources in render passes
		Data::AssetsStreamer& Assets;
		// Range slot used by renderer settings data CBuffer
		Binding::Range SettingsRange;
		// Range slot used by renderer dynamic data CBuffer
		Binding::Range DynamicDataRange;
		// Sampler definitions provided by the renderer. Can be extended with custom samplers too
		// if they don't overlap with main samplers (see Shader/Common/Samplers.hlsli)
		std::vector<Resource::SamplerDesc> Samplers;
		// Allows for caching same shader blobs between passes to speed up data loading
		std::unordered_map<std::string, Resource::Shader> ShaderCache;

		void FreeShaderCache(Device& dev) noexcept { for (auto& shader : ShaderCache) shader.second.Free(dev); }
	};

	// Main access point for all data to be used by pass provided by current renderer
	struct RendererPassExecuteData
	{
		// Alloc inside DynamicBuffers corresponding to DynamicData of the renderer
		static constexpr Resource::DynamicBufferAlloc RENDERER_DYNAMIC_BUFFER = { 0, 0 };

		FrameBuffer Buffers;
		Binding::Library Bindings;
		Resource::CBuffer SettingsBuffer;
		// Current dynamic constant buffer used for uploading data to GPU
		Ptr<Resource::DynamicCBuffer> DynamicBuffer;
		RendererSettingsData SettingsData;
		RendererDynamicData DynamicData;
		RendererGraphData GraphData;
		// Pointer to arbitrary data to be used by custom passes
		PtrVoid CustomData;

		void BindRendererDynamicData(CommandList& cl, Binding::Context& bindCtx) const noexcept { DynamicBuffer->Bind(cl, bindCtx, RENDERER_DYNAMIC_BUFFER); }
	};
}