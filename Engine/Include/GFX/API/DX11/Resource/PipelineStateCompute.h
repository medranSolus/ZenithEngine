#pragma once
#include "GFX/Resource/DataBinding.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class PipelineStateCompute final
	{
		DX::ComPtr<ID3D11ComputeShader> computeShader;

	public:
		PipelineStateCompute(GFX::Device& dev, GFX::Resource::Shader& shader, const GFX::Resource::DataBinding& binding);
		ZE_CLASS_MOVE(PipelineStateCompute);
		~PipelineStateCompute() = default;

		// Gfx API Internal

		void Bind(ID3D11DeviceContext4* ctx) const noexcept { ctx->CSSetShader(computeShader.Get(), nullptr, 0); }
	};
}