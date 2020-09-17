#pragma once
#include "IBufferResource.h"
#include "Surface.h"

namespace GFX::Pipeline::Resource
{
	class DepthStencil : public IBufferResource
	{
		friend class Graphics;
		friend class RenderTarget;
		friend class RenderTargetEx;

	public:
		enum class Usage { DepthStencil, DepthOnly };

	protected:
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView = nullptr;

		DepthStencil(Graphics& gfx, unsigned int width, unsigned int height, bool shaderResource, Usage usage = Usage::DepthStencil);

		static constexpr DXGI_FORMAT UsageTypeless(Usage usage) noexcept;
		static constexpr DXGI_FORMAT UsageTyped(Usage usage) noexcept;

	public:
		DepthStencil(Graphics& gfx, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, UINT size, UINT face);
		inline DepthStencil(Graphics& gfx, unsigned int width, unsigned int height, Usage usage = Usage::DepthStencil)
			: DepthStencil(gfx, width, height, false, usage) {}
		virtual ~DepthStencil() = default;

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->OMSetRenderTargets(0U, nullptr, depthStencilView.Get()); BindViewport(gfx); }
		inline void Clear(Graphics& gfx) noexcept override { GetContext(gfx)->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH | D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL, 1.0f, 0U); }

		Surface ToSurface(Graphics& gfx, bool linearScale = true) const;
	};
}