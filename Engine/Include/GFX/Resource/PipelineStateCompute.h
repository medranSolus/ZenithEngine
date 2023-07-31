#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Resource/PipelineStateCompute.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Resource/PipelineStateCompute.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Resource/PipelineStateCompute.h"
#endif

namespace ZE::GFX::Resource
{
	// GPU state for compute call
	class PipelineStateCompute final
	{
		ZE_RHI_BACKEND(Resource::PipelineStateCompute);

	public:
		PipelineStateCompute() = default;
		constexpr PipelineStateCompute(Device& dev, Shader& shader, const Binding::Schema& binding) { ZE_RHI_BACKEND_VAR.Init(dev, shader, binding); }
		ZE_CLASS_MOVE(PipelineStateCompute);
		~PipelineStateCompute() = default;

		constexpr void Init(Device& dev, Shader& shader, const Binding::Schema& binding) { ZE_RHI_BACKEND_VAR.Init(dev, shader, binding); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, Shader& shader, const Binding::Schema& binding) { ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, shader, binding); }
		ZE_RHI_BACKEND_GET(Resource::PipelineStateCompute);

		// Main Gfx API

		constexpr void Bind(CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(Bind, cl); }
		// Before destroying pipeline you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};
}