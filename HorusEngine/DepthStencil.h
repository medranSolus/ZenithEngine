#pragma once
#include "IBufferResource.h"

namespace GFX::Pipeline::Resource
{
	class DepthStencil : public IBufferResource
	{
		friend class Graphics;
		friend class RenderTarget;

	public:
		enum class Usage { DepthStencil, ShadowDepth };

	private:
		unsigned int width;
		unsigned int height;

	protected:
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView = nullptr;

		static inline DXGI_FORMAT UsageTypeless(Usage usage) noexcept;
		static inline DXGI_FORMAT UsageTyped(Usage usage) noexcept;

	public:
		DepthStencil(Graphics& gfx, unsigned int width, unsigned int height, Usage usage = Usage::DepthStencil);
		virtual ~DepthStencil() = default;

		constexpr unsigned int GetWidth() const noexcept { return width; }
		constexpr unsigned int GetHeight() const noexcept { return height; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->OMSetRenderTargets(0U, nullptr, depthStencilView.Get()); }
		inline void Clear(Graphics& gfx) noexcept override { GetContext(gfx)->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH | D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL, 1.0f, 0U); }
	};
}