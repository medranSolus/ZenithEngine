#pragma once
#include "IBufferResource.h"

namespace GFX::Pipeline::Resource
{
	class DepthStencil : public IBufferResource
	{
		friend class Graphics;
		friend class RenderTarget;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView = nullptr;

	public:
		DepthStencil(Graphics& gfx, unsigned int width, unsigned int height);
		virtual ~DepthStencil() = default;

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->OMSetRenderTargets(1U, nullptr, depthStencilView.Get()); }
		inline void Clear(Graphics& gfx) noexcept override { GetContext(gfx)->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH | D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL, 1.0f, 0U); }
	};
}