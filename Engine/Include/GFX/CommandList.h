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
		constexpr CommandList(Device& dev, QueueType type) { ZE_RHI_BACKEND_VAR.Init(dev, type); }
		ZE_CLASS_MOVE(CommandList);
		~CommandList() = default;

		constexpr bool IsInitialized() const noexcept { bool val = false; ZE_RHI_BACKEND_CALL_RET(val, IsInitialized); return val; }
		constexpr void InitMain(Device& dev) { ZE_RHI_BACKEND_VAR.Init(dev); }
		constexpr void Init(Device& dev, QueueType type = QueueType::Main) { ZE_RHI_BACKEND_VAR.Init(dev, type); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, QueueType type) { ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, type); }
		ZE_RHI_BACKEND_GET(CommandList);

		// Main Gfx API

		constexpr void Open(Device& dev) { ZE_RHI_BACKEND_CALL(Open, dev); }
		constexpr void Open(Device& dev, Resource::PipelineStateCompute& pso) { ZE_RHI_BACKEND_CALL(Open, dev, pso); }
		constexpr void Open(Device& dev, Resource::PipelineStateGfx& pso) { ZE_RHI_BACKEND_CALL(Open, dev, pso); }

		constexpr void Close(Device& dev) { ZE_RHI_BACKEND_CALL(Close, dev); }
		constexpr void Reset(Device& dev) { ZE_RHI_BACKEND_CALL(Reset, dev); }

		constexpr void DrawFullscreen(Device& dev) const noexcept(!_ZE_DEBUG_GFX_API) { ZE_RHI_BACKEND_CALL(DrawFullscreen, dev); }
		// For best performance each thread group should be multiple of 32 (ideally as many as 2*processors on GPU)
		constexpr void Compute(Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(!_ZE_DEBUG_GFX_API) { ZE_RHI_BACKEND_CALL(Compute, dev, groupX, groupY, groupZ); }

		// Before destroying command list you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }

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