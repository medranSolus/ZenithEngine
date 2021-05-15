#pragma once
#include "IRenderTarget.h"

namespace ZE::GFX::Pipeline::Resource
{
	class RenderTargetEx : public IRenderTarget
	{
		std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> targetViews;
		std::unique_ptr<ID3D11RenderTargetView* []> targetsArray = nullptr;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureViews;
		std::unique_ptr<ID3D11ShaderResourceView* []> texturesArray = nullptr;
		std::unique_ptr<ID3D11ShaderResourceView* []> nullTexturesArray = nullptr; // Maybe static
		U32 slot;
		U32 count;

	public:
		RenderTargetEx(Graphics& gfx, U32 width, U32 height, U32 slot, std::vector<DXGI_FORMAT>&& formats);
		virtual ~RenderTargetEx() = default;

		static GfxResPtr<RenderTargetEx> Get(Graphics& gfx, U32 slot, std::vector<DXGI_FORMAT>&& formats);

		void Bind(Graphics& gfx) const override { GetContext(gfx)->PSSetShaderResources(slot, count, texturesArray.get()); }
		void Unbind(Graphics& gfx) const override { UnbindAll(gfx); GetContext(gfx)->PSSetShaderResources(slot, count, nullTexturesArray.get()); }

		void BindTarget(Graphics& gfx) const override;
		void BindTarget(Graphics& gfx, DepthStencil& depthStencil) const override;

		void Clear(Graphics& gfx, const ColorF4& color) override;
		void Clear(Graphics& gfx, const std::vector<ColorF4>& colors);
		Surface ToSurface(Graphics& gfx) const override;
	};
}