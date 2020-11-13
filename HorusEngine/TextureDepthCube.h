#pragma once
#include "IRenderTarget.h"

namespace GFX::Resource
{
	class TextureDepthCube : public IBindable
	{
		static const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> nullShaderResource;

		UINT slot;
		UINT size;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
		GfxResPtr<Pipeline::Resource::IRenderTarget> depthBuffer;
		GfxResPtr<Pipeline::Resource::DepthStencil> stencil;

	public:
		TextureDepthCube(Graphics& gfx, UINT size, UINT slot = 0U);
		virtual ~TextureDepthCube() = default;

		static inline GfxResPtr<TextureDepthCube> Get(Graphics& gfx, UINT size, UINT slot = 0U);
		static inline std::string GenerateRID(UINT size, UINT slot = 0U) noexcept;

		inline GfxResPtr<Pipeline::Resource::IRenderTarget> GetBuffer() const noexcept { return depthBuffer; }
		inline GfxResPtr<Pipeline::Resource::DepthStencil> GetStencil() const noexcept { return stencil; }
		inline void Unbind(Graphics& gfx) noexcept { GetContext(gfx)->PSSetShaderResources(slot, 1U, nullShaderResource.GetAddressOf()); }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(size, slot); }
	};

	template<>
	struct is_resolvable_by_codex<TextureDepthCube>
	{
		static constexpr bool generate{ true };
	};

	inline GfxResPtr<TextureDepthCube> TextureDepthCube::Get(Graphics& gfx, UINT size, UINT slot)
	{
		return Codex::Resolve<TextureDepthCube>(gfx, size, slot);
	}

	inline std::string TextureDepthCube::GenerateRID(UINT size, UINT slot) noexcept
	{
		return "TD" + std::to_string(slot) + "#" + std::to_string(size);
	}
}