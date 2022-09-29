#pragma once
#include "IRenderTarget.h"

namespace ZE::GFX::Pipeline::Resource
{
	class RenderTarget : public IRenderTarget
	{
		static inline const Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> nullUAV = nullptr;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> targetView = nullptr;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav = nullptr;

	protected:
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		U32 slotUAV = UINT_MAX;

		RenderTarget(U32 width, U32 height, DXGI_FORMAT format, U32 slot, Graphics& gfx) noexcept
			: IRenderTarget(gfx, width, height), format(format), slotUAV(slot) {}

		Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture(Graphics& gfx, D3D11_TEXTURE2D_DESC& textureDesc);
		void InitializeTargetView(Graphics& gfx, D3D11_TEXTURE2D_DESC& textureDesc, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture);

	public:
		RenderTarget(Graphics& gfx, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, U32 slot = UINT_MAX);
		RenderTarget(Graphics& gfx, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, U32 size);
		RenderTarget(Graphics& gfx, U32 width, U32 height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, U32 slot = UINT_MAX);
		RenderTarget(Graphics& gfx, U32 width, U32 height, Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer, DXGI_FORMAT format);
		RenderTarget(RenderTarget&&) = default;
		RenderTarget& operator=(RenderTarget&&) = default;
		virtual ~RenderTarget() = default;

		void Release() noexcept { targetView = nullptr; }
		void BindCompute(Graphics& gfx) const override { BindComputeTarget(gfx); }
		void BindTarget(Graphics& gfx) const override { GetContext(gfx)->OMSetRenderTargets(1, targetView.GetAddressOf(), nullptr); BindViewport(gfx); }
		void BindComputeTarget(Graphics& gfx) const override { GetContext(gfx)->CSSetUnorderedAccessViews(slotUAV, 1, uav.GetAddressOf(), nullptr); }
		void Unbind(Graphics& gfx) const override { IRenderTarget::Unbind(gfx); if (slotUAV != UINT_MAX) GetContext(gfx)->CSSetUnorderedAccessViews(slotUAV, 1, nullUAV.GetAddressOf(), nullptr); }

#ifdef _ZE_MODE_DEBUG
		std::string GetRID() const noexcept override;
#endif
		void BindTarget(Graphics& gfx, DepthStencil& depthStencil) const override;
		void Clear(Graphics& gfx, const ColorF4& color) override;
		Surface ToSurface(Graphics& gfx) const override;
	};
}