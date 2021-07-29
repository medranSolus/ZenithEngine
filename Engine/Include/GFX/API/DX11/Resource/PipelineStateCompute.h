#pragma once
#include "GFX/Context.h"
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

		void Bind(GFX::Context& ctx) const noexcept;
	};
}