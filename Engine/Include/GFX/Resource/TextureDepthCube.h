#pragma once
#include "GFX/Pipeline/Resource/IRenderTarget.h"

namespace ZE::GFX::Resource
{
	class TextureDepthCube : public IBindable
	{
		static const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> nullShaderResource;

		U32 slot;
		U32 size;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
		GfxResPtr<Pipeline::Resource::IRenderTarget> depthBuffer;
		GfxResPtr<Pipeline::Resource::DepthStencil> stencil;

	public:
		TextureDepthCube(Graphics& gfx, U32 size, U32 slot = 0);
		virtual ~TextureDepthCube() = default;

		static std::string GenerateRID(U32 size, U32 slot = 0) noexcept;
		static GfxResPtr<TextureDepthCube> Get(Graphics& gfx, U32 size, U32 slot = 0);

		GfxResPtr<Pipeline::Resource::IRenderTarget> GetBuffer() const noexcept { return depthBuffer; }
		GfxResPtr<Pipeline::Resource::DepthStencil> GetStencil() const noexcept { return stencil; }
		void Unbind(Graphics& gfx) const noexcept { GetContext(gfx)->PSSetShaderResources(slot, 1, nullShaderResource.GetAddressOf()); }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->PSSetShaderResources(slot, 1, textureView.GetAddressOf()); }
		void BindCompute(Graphics& gfx) const override { GetContext(gfx)->CSSetShaderResources(slot, 1, textureView.GetAddressOf()); }
		std::string GetRID() const noexcept override { return GenerateRID(size, slot); }
	};

	template<>
	struct is_resolvable_by_codex<TextureDepthCube>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}