#pragma once
#include "GFX/Resource/DataBinding.h"
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
		PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Resource::DataBinding& binding);
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() = default;

		// Gfx API Internal

		void Bind(ID3D11DeviceContext4* ctx) const noexcept;
	};
}