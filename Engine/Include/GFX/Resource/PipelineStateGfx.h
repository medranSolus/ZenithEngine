#pragma once
#include "GFX/API/DX11/Resource/PipelineStateGfx.h"
#include "GFX/API/DX12/Resource/PipelineStateGfx.h"

namespace ZE::GFX::Resource
{
	// GPU state for draw call
	class PipelineStateGfx final
	{
		ZE_API_BACKEND(Resource::PipelineStateGfx);

	public:
		PipelineStateGfx() = default;
		constexpr PipelineStateGfx(Device& dev, const PipelineStateDesc& desc, const Material::Schema& binding) { ZE_API_BACKEND_VAR.Init(dev, desc, binding); }
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() = default;

		constexpr void Init(Device& dev, const PipelineStateDesc& desc, const Material::Schema& binding) { ZE_API_BACKEND_VAR.Init(dev, desc, binding); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const PipelineStateDesc& desc, Material::Schema& binding) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, desc, binding); }
		ZE_API_BACKEND_GET(Resource::PipelineStateGfx);
	};
}