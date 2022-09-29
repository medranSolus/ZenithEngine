#pragma once
#include "GFX/Resource/Viewport.h"
#include "GFX/Surface.h"

namespace ZE::GFX::Pipeline::Resource
{
	class IBufferResource : public GFX::Resource::IBindable
	{
		GFX::Resource::Viewport viewport;

	protected:
		static inline const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> nullShaderResource = nullptr;

		void BindViewport(Graphics& gfx) const { viewport.Bind(gfx); }

	public:
		IBufferResource(GFX::Resource::Viewport&& viewport) noexcept : viewport(std::move(viewport)) {}
		IBufferResource(Graphics& gfx, U32 width, U32 height) noexcept
			: viewport(gfx, width, height) {}
		IBufferResource(IBufferResource&&) = default;
		IBufferResource& operator=(IBufferResource&&) = default;
		virtual ~IBufferResource() = default;

		constexpr U32 GetWidth() const noexcept { return viewport.GetWidth(); }
		constexpr U32 GetHeight() const noexcept { return viewport.GetHeight(); }

		virtual void Unbind(Graphics& gfx) const { GetContext(gfx)->OMSetRenderTargets(0, nullptr, nullptr); }
		virtual void Clear(Graphics& gfx, const ColorF4& color) { Clear(gfx); }
		virtual void Clear(Graphics& gfx) = 0;

		virtual Surface ToSurface(Graphics& gfx) const = 0;
	};
}