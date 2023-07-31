#pragma once
#include "GFX/Binding/Schema.h"

namespace ZE::RHI::DX11::Resource
{
	class PipelineStateGfx final
	{
		D3D_PRIMITIVE_TOPOLOGY topology;
		DX::ComPtr<IInputLayout> inputLayout;
		DX::ComPtr<IVertexShader> vertexShader;
		DX::ComPtr<IDomainShader> domainShader;
		DX::ComPtr<IHullShader> hullShader;
		DX::ComPtr<IGeometryShader> geometryShader;
		DX::ComPtr<IPixelShader> pixelShader;
		DX::ComPtr<IBlendState> blendState;
		DX::ComPtr<IDepthStencilState> depthStencilState;
		DX::ComPtr<IRasterizerState> rasterState;

	public:
		PipelineStateGfx() = default;
		PipelineStateGfx(GFX::Device& dev, const GFX::Resource::PipelineStateDesc& desc, const GFX::Binding::Schema& binding);
		ZE_CLASS_MOVE(PipelineStateGfx);
		~PipelineStateGfx() { ZE_ASSERT(vertexShader == nullptr, "Pipeline not freed before deletion!"); }

		void SetStencilRef(GFX::CommandList& cl, U32 refValue) const noexcept { SetStencilRef(cl.Get().dx11.GetContext(), refValue); }
		void Bind(GFX::CommandList& cl) const noexcept { Bind(cl.Get().dx11.GetContext()); }
		void Free(GFX::Device& dev) noexcept;

		// Gfx API Internal

		void SetStencilRef(IDeviceContext* ctx, U32 refValue) const noexcept { ctx->OMSetDepthStencilState(depthStencilState.Get(), refValue); }
		void Bind(IDeviceContext* ctx) const noexcept;
	};
}