#pragma once
#include "Codex.h"

namespace GFX::Pipeline::Resource
{
	class DepthStencil;
}

namespace GFX::Resource
{
	class TextureDepthCube : public IBindable
	{
		static const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> nullShaderResource;

		UINT slot;
		UINT size;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
		std::vector<std::shared_ptr<Pipeline::Resource::DepthStencil>> depthBuffers;

	public:
		TextureDepthCube(Graphics& gfx, UINT size, UINT slot = 0U);
		virtual ~TextureDepthCube() = default;

		static inline std::shared_ptr<TextureDepthCube> Get(Graphics& gfx, UINT size, UINT slot = 0U);
		static inline std::string GenerateRID(UINT size, UINT slot = 0U) noexcept;

		inline std::shared_ptr<Pipeline::Resource::DepthStencil> GetBuffer(size_t index) noexcept { return depthBuffers.at(index); }
		inline void Unbind(Graphics& gfx) noexcept { GetContext(gfx)->PSSetShaderResources(slot, 1U, nullShaderResource.GetAddressOf()); }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(size, slot); }
	};

	template<>
	struct is_resolvable_by_codex<TextureDepthCube>
	{
		static constexpr bool generate{ true };
	};

	inline std::shared_ptr<TextureDepthCube> TextureDepthCube::Get(Graphics& gfx, UINT size, UINT slot)
	{
		return Codex::Resolve<TextureDepthCube>(gfx, size, slot);
	}

	inline std::string TextureDepthCube::GenerateRID(UINT size, UINT slot) noexcept
	{
		return "#" + std::string(typeid(TextureDepthCube).name()) + "#" + std::to_string(size) + "#" + std::to_string(slot) + "#";
	}
}