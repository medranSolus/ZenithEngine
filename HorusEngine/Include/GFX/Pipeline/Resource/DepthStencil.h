#pragma once
#include "IBufferResource.h"

namespace GFX::Pipeline::Resource
{
	class DepthStencil : public IBufferResource
	{
		friend class Graphics;
		friend class RenderTarget;
		friend class RenderTargetEx;

	public:
		enum class Usage : bool { DepthStencil, DepthOnly };

	protected:
		Usage usage;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView = nullptr;

		static constexpr DXGI_FORMAT UsageTypeless(Usage usage) noexcept;
		static constexpr DXGI_FORMAT UsageTyped(Usage usage) noexcept;

		DepthStencil(Graphics& gfx, U32 width, U32 height, bool shaderResource, Usage usage = Usage::DepthStencil);

	public:
		DepthStencil(Graphics& gfx, U32 width, U32 height, Usage usage = Usage::DepthStencil)
			: DepthStencil(gfx, width, height, false, usage) {}
		DepthStencil(Graphics& gfx, Usage usage = Usage::DepthStencil);
		DepthStencil(Graphics& gfx, U32 size);
		virtual ~DepthStencil() = default;

		void Bind(Graphics& gfx) const override { GetContext(gfx)->OMSetRenderTargets(0, nullptr, depthStencilView.Get()); BindViewport(gfx); }
		void Clear(Graphics& gfx) override { GetContext(gfx)->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); }
		Surface ToSurface(Graphics& gfx) const override { return ToSurface(gfx, true); }

#ifdef _MODE_DEBUG
		std::string GetRID() const noexcept override;
#endif
		Surface ToSurface(Graphics& gfx, bool linearScale) const;
	};
}