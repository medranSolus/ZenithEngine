#pragma once
#include "GFX/API/DX11/Resource/PipelineStateGfx.h"
#include "GFX/Context.h"

namespace ZE::GFX::Resource
{
	// GPU state for draw call
	class PipelineStateGfx final
	{
		ZE_API_BACKEND(PipelineStateGfx, DX11, DX11, DX11, DX11) backend;

	public:
		constexpr PipelineStateGfx(Device& dev, const PipelineStateDesc& desc) { backend.Init(dev, desc); }
		PipelineStateGfx(PipelineStateGfx&&) = delete;
		PipelineStateGfx(const PipelineStateGfx&) = delete;
		PipelineStateGfx& operator=(PipelineStateGfx&&) = delete;
		PipelineStateGfx& operator=(const PipelineStateGfx&) = delete;
		~PipelineStateGfx() = default;

		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const PipelineStateDesc& desc) { backend.Switch(nextApi, dev, desc); }
		constexpr ZE_API_BACKEND(PipelineStateGfx, DX11, DX11, DX11, DX11)& Get() noexcept { return backend; }

		void Bind(Context& ctx) const noexcept { ZE_API_BACKEND_CALL(backend, Bind, ctx); }
	};
}