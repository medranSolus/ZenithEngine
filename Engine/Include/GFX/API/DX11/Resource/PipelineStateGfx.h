#pragma once
#include "GFX/Context.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class PipelineStateGfx final
	{
		DX::ComPtr<ID3D11InputLayout> inputLayout;
		DX::ComPtr<ID3D11VertexShader> vertexShader;
		DX::ComPtr<ID3D11DomainShader> domainShader;
		DX::ComPtr<ID3D11HullShader> hullShader;
		DX::ComPtr<ID3D11GeometryShader> geometryShader;
		DX::ComPtr<ID3D11PixelShader> pixelShader;
		DX::ComPtr<ID3D11BlendState> blendState;
		DX::ComPtr<ID3D11DepthStencilState> depthStencilState;
		DX::ComPtr<ID3D11RasterizerState> rasterState;

	public:
		PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc);
		PipelineStateGfx(PipelineStateGfx&&) = delete;
		PipelineStateGfx(const PipelineStateGfx&) = delete;
		PipelineStateGfx& operator=(PipelineStateGfx&&) = delete;
		PipelineStateGfx& operator=(const PipelineStateGfx&) = delete;
		~PipelineStateGfx() = default;

		void Bind(GFX::Context& ctx) const noexcept;
	};
}