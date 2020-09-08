#pragma once
#include "Viewport.h"

namespace GFX::Pipeline::Resource
{
	class IBufferResource : public GFX::Resource::IBindable
	{
		std::shared_ptr<GFX::Resource::Viewport> viewport = nullptr;

	protected:
		static const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> nullShaderResource;

		inline void BindViewport(Graphics& gfx) noexcept { viewport->Bind(gfx); }

	public:
		inline IBufferResource(std::shared_ptr<GFX::Resource::Viewport> viewport) noexcept : viewport(std::move(viewport)) {}
		IBufferResource(Graphics& gfx, unsigned int width, unsigned int height) noexcept;
		virtual ~IBufferResource() = default;

		static inline void UnbindAll(Graphics& gfx) noexcept { GetContext(gfx)->OMSetRenderTargets(0U, nullptr, nullptr); }

		inline unsigned int GetWidth() const noexcept { return viewport->GetWidth(); }
		inline unsigned int GetHeight() const noexcept { return viewport->GetHeight(); }

		virtual inline void Unbind(Graphics& gfx) noexcept { UnbindAll(gfx); }
		virtual void Clear(Graphics& gfx) noexcept = 0;
		virtual inline void Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept { Clear(gfx); }
		inline std::string GetRID() const noexcept override { return "?"; }
	};
}