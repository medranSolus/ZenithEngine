#pragma once
#include "GFX/API/DX11/Resource/PipelineStateGfx.h"
#include "GFX/API/DX12/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Resource
{
	// GPU state for draw call
	class PipelineStateGfx final
	{
		ZE_API_BACKEND(Resource::PipelineStateGfx) backend;

	public:
		constexpr PipelineStateGfx(Device& dev, const PipelineStateDesc& desc) { backend.Init(dev, desc); }
		PipelineStateGfx(PipelineStateGfx&&) = default;
		PipelineStateGfx(const PipelineStateGfx&) = delete;
		PipelineStateGfx& operator=(PipelineStateGfx&&) = default;
		PipelineStateGfx& operator=(const PipelineStateGfx&) = delete;
		~PipelineStateGfx() = default;

		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const PipelineStateDesc& desc) { backend.Switch(nextApi, dev, desc); }
		constexpr ZE_API_BACKEND(Resource::PipelineStateGfx)& Get() noexcept { return backend; }
	};
}