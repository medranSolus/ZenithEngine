#pragma once
#include "Viewport.h"

namespace GFX::Pipeline::Resource
{
	class IBufferResource : public GFX::Resource::IBindable
	{
		GfxResPtr<GFX::Resource::Viewport> viewport;

	protected:
		static const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> nullShaderResource;

		inline void BindViewport(Graphics& gfx) { viewport->Bind(gfx); }

	public:
		inline IBufferResource(GfxResPtr<GFX::Resource::Viewport>&& viewport) noexcept : viewport(std::move(viewport)) {}
		inline IBufferResource(Graphics& gfx, unsigned int width, unsigned int height) noexcept
			: viewport(GFX::Resource::Viewport::Get(gfx, width, height)) {}
		virtual ~IBufferResource() = default;

		static inline void UnbindAll(Graphics& gfx) noexcept { GetContext(gfx)->OMSetRenderTargets(0U, nullptr, nullptr); }

		constexpr unsigned int GetWidth() const noexcept { return viewport->GetWidth(); }
		constexpr unsigned int GetHeight() const noexcept { return viewport->GetHeight(); }

		virtual inline void Unbind(Graphics& gfx) noexcept { UnbindAll(gfx); }
		virtual void Clear(Graphics& gfx) noexcept = 0;
		virtual inline void Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept { Clear(gfx); }
		inline std::string GetRID() const noexcept override { return IBindable::GetNoCodexRID(); }
	};
}