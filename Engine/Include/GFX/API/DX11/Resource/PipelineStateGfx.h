#pragma once
#include "GFX/Binding/Schema.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Resource
{
	class PipelineStateGfx final
	{
		D3D_PRIMITIVE_TOPOLOGY topology;
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
		PipelineStateGfx() = default;
		PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() = default;

		void Bind(GFX::CommandList& cl) const noexcept { Bind(cl.Get().dx11.GetContext()); }

		// Gfx API Internal

		void Bind(ID3D11DeviceContext4* ctx) const noexcept;
	};
}