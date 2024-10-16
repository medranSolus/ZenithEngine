#include "GFX/Pipeline/RenderGraphDesc.h"

namespace ZE::GFX::Pipeline
{
	PixelFormat RenderGraphDesc::GetFormat(const std::string& name) const noexcept
	{
		auto it = Resources.find(name);
		return it != Resources.end() ? it->second.Format : PixelFormat::Unknown;
	}

	void RenderGraphDesc::InitBuffers() noexcept
	{
		// Backbuffer
		AddResource(BACKBUFFER_NAME,
			{
				{ 1, 1 }, 1, FrameResourceFlag::None | FrameResourceFlag::SyncDisplaySize,
				Settings::BackbufferFormat, ColorF4(0.0f, 0.0f, 0.0f, 1.0f)
			});
	}

	void RenderGraphDesc::AddResource(std::string_view name, FrameResourceDesc&& desc) noexcept
	{
		ZE_ASSERT(!((desc.Flags & FrameResourceFlag::ForceDSV) && (desc.Flags & FrameResourceFlag::SimultaneousAccess)),
			"Cannot use depth stencil with simultaneous access resource!");
		ZE_ASSERT(!((desc.Flags & FrameResourceFlag::SyncRenderSize) && (desc.Flags & FrameResourceFlag::SyncDisplaySize)),
			"Cannot sync same resource to both render and display sizes!");
		ZE_ASSERT(Resources.size() < INVALID_RID, "Too much resources, needed wider type!");
		ZE_ASSERT(!Resources.contains(std::string(name)), "Resource with same name is already present!");

		Resources.emplace(name, std::forward<FrameResourceDesc>(desc));
	}
}