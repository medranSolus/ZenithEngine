#pragma once
#include "GFX/Binding/Schema.h"

namespace ZE::RHI::DX11::Resource
{
	class PipelineStateCompute final
	{
		DX::ComPtr<IComputeShader> computeShader;

	public:
		PipelineStateCompute() = default;
		ZE_CLASS_MOVE(PipelineStateCompute);
		~PipelineStateCompute() = default;

		static Expected<PipelineStateCompute> Create(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Binding::Schema& binding) noexcept;

		void Bind(GFX::CommandList& cl) const noexcept { Bind(cl.Get().dx11.GetContext()); }

		// Gfx API Internal

		void Bind(IDeviceContext* ctx) const noexcept { ctx->CSSetShader(computeShader.Get(), nullptr, 0); }
	};
}