#pragma once
#include "GFX/Device.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class PipelineStateCompute final
	{
		DX::ComPtr<ID3D11ComputeShader> shader;

	public:
		PipelineStateCompute(GFX::Device& dev, const std::wstring& nameCS);
		PipelineStateCompute(PipelineStateCompute&&) = default;
		PipelineStateCompute(const PipelineStateCompute&) = delete;
		PipelineStateCompute& operator=(PipelineStateCompute&&) = default;
		PipelineStateCompute& operator=(const PipelineStateCompute&) = delete;
		~PipelineStateCompute() = default;

		// Gfx API Internal

		void Bind(ID3D11DeviceContext4* ctx) const noexcept { ctx->CSSetShader(shader.Get(), nullptr, 0); }
	};
}