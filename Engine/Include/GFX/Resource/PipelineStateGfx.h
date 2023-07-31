#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/Resource/PipelineStateGfx.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/Resource/PipelineStateGfx.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/Resource/PipelineStateGfx.h"
#endif

namespace ZE::GFX::Resource
{
	// GPU state for draw call
	class PipelineStateGfx final
	{
		ZE_RHI_BACKEND(Resource::PipelineStateGfx);

	public:
		PipelineStateGfx() = default;
		constexpr PipelineStateGfx(Device& dev, const PipelineStateDesc& desc, const Binding::Schema& binding) { ZE_RHI_BACKEND_VAR.Init(dev, desc, binding); }
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() = default;

		constexpr void Init(Device& dev, const PipelineStateDesc& desc, const Binding::Schema& binding) { ZE_RHI_BACKEND_VAR.Init(dev, desc, binding); }
		constexpr void SwitchApi(GfxApiType nextApi, Device& dev, const PipelineStateDesc& desc, Binding::Schema& binding) { ZE_RHI_BACKEND_VAR.Switch(nextApi, dev, desc, binding); }
		ZE_RHI_BACKEND_GET(Resource::PipelineStateGfx);

		// Main Gfx API

		constexpr void SetStencilRef(CommandList& cl, U32 refValue) const noexcept { ZE_RHI_BACKEND_CALL(SetStencilRef, cl, refValue); }
		constexpr void Bind(CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(Bind, cl); }
		// Before destroying pipeline you have to call this function for proper memory freeing
		constexpr void Free(Device& dev) noexcept { ZE_RHI_BACKEND_CALL(Free, dev); }
	};
}