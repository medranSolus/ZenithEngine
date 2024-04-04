#pragma once
#include "FrameResourceDesc.h"
#include "RenderNode.h"

namespace ZE::GFX::Pipeline
{
	// User-filled description of whole render graph with render passes and framebuffer resources
	// Also user provided callbacks should be here for various parts of render graph
	// (ex. handling of data) as well as pointers to custom data structures
	struct RenderGraphDesc
	{
		std::vector<FrameResourceDesc> Resources;
		std::vector<RenderNode> RenderPasses;
		std::vector<Resource::SamplerDesc> Samplers;
		Binding::Range SettingsRange;
		Binding::Range DynamicDataRange;
		RendererSettingsData SettingsData;
		// TODO: here comes all the custom callbacks and pointers

		constexpr PixelFormat GetFormat(RID id) const noexcept { return id == INVALID_RID ? PixelFormat::Unknown : Resources.at(id).Format; }

		constexpr void InitBuffers(RID resourceCount) noexcept;
		constexpr RID AddResource(FrameResourceDesc&& desc) noexcept;
	};

#pragma region Functions
	constexpr void RenderGraphDesc::InitBuffers(RID resourceCount) noexcept
	{
		Resources.reserve(resourceCount + 1);
		// Backbuffer
		AddResource(
			{
				Settings::DisplaySize, 1, FrameResourceFlag::None | FrameResourceFlag::SyncDisplaySize,
				Settings::BackbufferFormat, ColorF4(0.0f, 0.0f, 0.0f, 1.0f)
			});
	}

	constexpr RID RenderGraphDesc::AddResource(FrameResourceDesc&& desc) noexcept
	{
		ZE_ASSERT(!((desc.Flags & FrameResourceFlag::ForceDSV) && (desc.Flags & FrameResourceFlag::SimultaneousAccess)),
			"Cannot use depth stencil with simultaneous access resource!");
		ZE_ASSERT(!((desc.Flags & FrameResourceFlag::SyncRenderSize) && (desc.Flags & FrameResourceFlag::SyncDisplaySize)),
			"Cannot sync same resource to both render and display sizes!");
		RID id = Utils::SafeCast<RID>(Resources.size());
		Resources.emplace_back(std::forward<FrameResourceDesc>(desc));
		return id;
	}
#pragma endregion
}