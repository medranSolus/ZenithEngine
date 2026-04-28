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
		ZE_CLASS_MOVE(PipelineStateCompute);
		~PipelineStateCompute() = default;

		static Expected<PipelineStateCompute> Create(Device& dev, Shader& shader, const Binding::Schema& binding) noexcept { ZE_RHI_BACKEND_CREATE(Resource::PipelineStateCompute, dev, shader, binding); }
		ZE_RHI_BACKEND_GET(Resource::PipelineStateCompute);

		// Main Gfx API

		constexpr void Bind(CommandList& cl) const noexcept { ZE_RHI_BACKEND_CALL(Bind, cl); }
	};
}