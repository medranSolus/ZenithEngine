#pragma once
#include "API/DX11/CommandList.h"
#include "API/DX12/CommandList.h"
#include "API/VK/CommandList.h"
#include "API/Backend.h"

namespace ZE::GFX
{
	// Storing commands for GPU
	class CommandList final
	{
		ZE_API_BACKEND(CommandList);

	public:
		CommandList() = default;
		constexpr CommandList(Device& dev, CommandType type) { ZE_API_BACKEND_VAR.Init(dev, type); }
		ZE_CLASS_MOVE(CommandList);
		~CommandList() = default;

		constexpr void InitMain(Device& dev) { ZE_API_BACKEND_VAR.Init(dev); }
		constexpr void Init(Device& dev, CommandType type = CommandType::All) { ZE_API_BACKEND_VAR.Init(dev, type); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, CommandType type) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, type); }
		ZE_API_BACKEND_GET(CommandList);

		// Main Gfx API

#ifdef _ZE_MODE_DEBUG
		constexpr void TagBegin(GFX::Device& dev, const wchar_t* tag, Pixel color) const noexcept { if (Settings::GfxTagsActive()) { ZE_API_BACKEND_CALL(TagBegin, dev, tag, color); } }
		constexpr void TagEnd(GFX::Device& dev) const noexcept { if (Settings::GfxTagsActive()) { ZE_API_BACKEND_CALL(TagEnd, dev); } }
#endif
		constexpr void Open(Device& dev) { ZE_API_BACKEND_CALL(Open, dev); }
		constexpr void Open(Device& dev, Resource::PipelineStateCompute& pso) { ZE_API_BACKEND_CALL(Open, dev, pso); }
		constexpr void Open(Device& dev, Resource::PipelineStateGfx& pso) { ZE_API_BACKEND_CALL(Open, dev, pso); }

		constexpr void Close(Device& dev) { ZE_API_BACKEND_CALL(Close, dev); }
		constexpr void Reset(Device& dev) { ZE_API_BACKEND_CALL(Reset, dev); }

		constexpr void Draw(Device& dev, U32 vertexCount) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(Draw, dev, vertexCount); }
		constexpr void DrawIndexed(Device& dev, U32 indexCount) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(DrawIndexed, dev, indexCount); }
		constexpr void DrawFullscreen(Device& dev) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(DrawFullscreen, dev); }
		// For best performance each thread group should be multiple of 32 (ideally as many as 2*processors on GPU)
		constexpr void Compute(Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(Compute, dev, groupX, groupY, groupZ); }
	};
}

#ifdef _ZE_MODE_DEBUG
#define ZE_DRAW_TAG_BEGIN(dev, cl, tag, color) cl.TagBegin(dev, tag, color)
#define ZE_DRAW_TAG_END(dev, cl) cl.TagEnd(dev)
#else
#define ZE_DRAW_TAG_BEGIN(dev, cl, tag, color)
#define ZE_DRAW_TAG_END(dev, cl)
#endif