#pragma once
#include "GFX/Binding/Schema.h"

namespace ZE::GFX::API::DX11::Resource
{
	class PipelineStateCompute final
	{
		DX::ComPtr<ID3D11ComputeShader> computeShader;

	public:
		PipelineStateCompute() = default;
		PipelineStateCompute(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Binding::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateCompute);
		~PipelineStateCompute() = default;

		void Bind(GFX::CommandList& cl) const noexcept { Bind(cl.Get().dx11.GetContext()); }

		// Gfx API Internal

		void Bind(ID3D11DeviceContext4* ctx) const noexcept { ctx->CSSetShader(computeShader.Get(), nullptr, 0); }
	};
}