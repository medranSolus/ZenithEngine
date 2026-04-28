#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/CommandList.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/CommandList.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/CommandList.h"
#endif
#include "Device.h"

namespace ZE::GFX
{
	// Storing commands for GPU
	class CommandList final
	{
		ZE_RHI_BACKEND(CommandList);

	public:
		CommandList() = default;
		ZE_CLASS_MOVE(CommandList);
		~CommandList() = default;

		static Expected<CommandList> CreateMain(Device& dev) noexcept { ZE_RHI_BACKEND_CREATE(CommandList, dev); }
		static Expected<CommandList> Create(Device& dev, GFX::QueueType type = QueueType::Main) noexcept { ZE_RHI_BACKEND_CREATE(CommandList, dev, type); }
		ZE_RHI_BACKEND_GET(CommandList);

		// Main Gfx API

		constexpr bool IsInitialized() const noexcept { ZE_RHI_BACKEND_CALL_RET(IsInitialized); }
		constexpr void* GetHandle() const noexcept { ZE_RHI_BACKEND_CALL_RET(GetHandle); }

		Status Open(Device& dev) const noexcept { ZE_RHI_BACKEND_CALL_RET(Open, dev); }
		Status Open(Device& dev, Resource::PipelineStateCompute& pso) const noexcept { ZE_RHI_BACKEND_CALL_RET(Open, dev, pso); }
		Status Open(Device& dev, Resource::PipelineStateGfx& pso) const noexcept { ZE_RHI_BACKEND_CALL_RET(Open, dev, pso); }

		// After using command list for external libraries with their own set of memory it is necessary to restore original memory state.
		// Note that this not include actual pipeline states set for this command list
		constexpr void RestoreExternalState(GFX::Device& dev) const noexcept { ZE_RHI_BACKEND_CALL(RestoreExternalState, dev); }

		Status Close(Device& dev) const noexcept { ZE_RHI_BACKEND_CALL_RET(Close, dev); }
		Status Reset(Device& dev) const noexcept { ZE_RHI_BACKEND_CALL_RET(Reset, dev); }

		constexpr void DrawFullscreen(Device& dev) const noexcept { ZE_RHI_BACKEND_CALL(DrawFullscreen, dev); }
		// For best performance each thread group should be multiple of 32 (ideally as many as 2*processors on GPU)
		constexpr void Compute(Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept { ZE_RHI_BACKEND_CALL(Compute, dev, groupX, groupY, groupZ); }

		constexpr void WriteBreadcrumbs(Device& dev, U32 value, U64 location, void* breadcrumbsBuffer, bool isBegin) const noexcept { ZE_RHI_BACKEND_CALL(WriteBreadcrumbs, dev, value, location, breadcrumbsBuffer, isBegin); }

#if _ZE_GFX_MARKERS
		constexpr void TagBegin(GFX::Device& dev, std::string_view tag, Pixel color) const noexcept;
		constexpr void TagEnd(GFX::Device& dev) const noexcept;
#endif
	};

#pragma region Functions
#if _ZE_GFX_MARKERS
	constexpr void CommandList::TagBegin(GFX::Device& dev, std::string_view tag, Pixel color) const noexcept
	{
		if (Settings::IsEnabledGfxTags())
		{
			ZE_RHI_BACKEND_CALL(TagBegin, dev, tag, color);
		}
	}

	constexpr void CommandList::TagEnd(GFX::Device& dev) const noexcept
	{
		if (Settings::IsEnabledGfxTags())
		{
			ZE_RHI_BACKEND_CALL(TagEnd, dev);
		}
	}
#endif
#pragma endregion
}

#if _ZE_GFX_MARKERS
#	define ZE_DRAW_TAG_BEGIN(dev, cl, tag, color) cl.TagBegin(dev, tag, color)
#	define ZE_DRAW_TAG_END(dev, cl) cl.TagEnd(dev)
#else
#	define ZE_DRAW_TAG_BEGIN(dev, cl, tag, color)
#	define ZE_DRAW_TAG_END(dev, cl)
#endif