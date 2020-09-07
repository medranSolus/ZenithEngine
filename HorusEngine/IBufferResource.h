#pragma once
#include "IBindable.h"

namespace GFX::Pipeline::Resource
{
	class IBufferResource : public GFX::Resource::IBindable
	{
		D3D11_VIEWPORT viewport = { 0 };
		unsigned int width;
		unsigned int height;

	protected:
		static const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> nullShaderResource;

		inline void BindViewport(Graphics& gfx) noexcept { GetContext(gfx)->RSSetViewports(1U, &viewport); }

	public:
		IBufferResource(unsigned int width, unsigned int height) noexcept;
		virtual ~IBufferResource() = default;

		static inline void UnbindAll(Graphics& gfx) noexcept { GetContext(gfx)->OMSetRenderTargets(0U, nullptr, nullptr); }

		constexpr unsigned int GetWidth() const noexcept { return width; }
		constexpr unsigned int GetHeight() const noexcept { return height; }

		virtual inline void Unbind(Graphics& gfx) noexcept { UnbindAll(gfx); }
		virtual void Clear(Graphics& gfx) noexcept = 0;
		virtual inline void Clear(Graphics& gfx, const Data::ColorFloat4& color) noexcept { Clear(gfx); }
		inline std::string GetRID() const noexcept override { return "?"; }
	};
}