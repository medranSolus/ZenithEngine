#pragma once
#include "IRenderTarget.h"

namespace GFX::Pipeline::Resource
{
	class RenderTargetEx : public IRenderTarget
	{
		std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> targetViews;
		std::unique_ptr<ID3D11RenderTargetView* []> targetsArray = nullptr;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureViews;
		std::unique_ptr<ID3D11ShaderResourceView* []> texturesArray = nullptr;
		std::unique_ptr<ID3D11ShaderResourceView* []> nullTexturesArray = nullptr; // Maybe static
		UINT slot;
		UINT count;

	public:
		RenderTargetEx(Graphics& gfx, unsigned int width, unsigned int height, UINT slot, std::vector<DXGI_FORMAT>&& formats);
		virtual ~RenderTargetEx() = default;

		static GfxResPtr<RenderTargetEx> Get(Graphics& gfx, UINT slot, std::vector<DXGI_FORMAT>&& formats);

		inline void BindTarget(Graphics& gfx) override { GetContext(gfx)->OMSetRenderTargets(count, targetsArray.get(), nullptr); BindViewport(gfx); }
		inline void BindTarget(Graphics& gfx, DepthStencil& depthStencil) override { GetContext(gfx)->OMSetRenderTargets(count, targetsArray.get(), depthStencil.depthStencilView.Get()); BindViewport(gfx); }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetShaderResources(slot, count, texturesArray.get()); }
		inline void Unbind(Graphics& gfx) noexcept override { UnbindAll(gfx); GetContext(gfx)->PSSetShaderResources(slot, count, nullTexturesArray.get()); }

		void Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept override;
		void Clear(Graphics& gfx, const std::vector<Data::ColorFloat4>& colors) noexcept;
		Surface ToSurface(Graphics& gfx) const override;
	};
}