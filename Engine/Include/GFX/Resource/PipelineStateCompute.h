#pragma once
#include "GFX/API/DX11/Resource/PipelineStateCompute.h"
#include "GFX/API/DX12/Resource/PipelineStateCompute.h"

namespace ZE::GFX::Resource
{
	// GPU state for compute call
	class PipelineStateCompute final
	{
		ZE_API_BACKEND(Resource::PipelineStateCompute);

	public:
		constexpr PipelineStateCompute(Device& dev, Shader& shader, const DataBinding& binding) { ZE_API_BACKEND_VAR.Init(dev, shader, binding); }
		ZE_CLASS_MOVE(PipelineStateCompute);
		~PipelineStateCompute() = default;

		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, Shader& shader, const DataBinding& binding) { ZE_API_BACKEND_VAR.Switch(nextApi, dev, shader, binding); }
		ZE_API_BACKEND_GET(Resource::PipelineStateCompute);
	};
}