#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class Texture : public IBindable
	{
		UINT slot;
		std::string path;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

	public:
		Texture(Graphics & gfx, const std::string & path, UINT slot = 0U);
		Texture(const Texture&) = delete;
		Texture & operator=(const Texture&) = delete;
		~Texture() = default;

		static inline std::shared_ptr<Texture> Get(Graphics & gfx, const std::string & path, UINT slot = 0U);
		static inline std::string GenerateRID(const std::string & path, UINT slot = 0U) noexcept;

		inline void Bind(Graphics & gfx) noexcept override { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(path, slot); }
	};

	template<>
	struct is_resolvable_by_codex<Texture>
	{
		static constexpr bool value{ true };
	};

	inline std::shared_ptr<Texture> Texture::Get(Graphics & gfx, const std::string & path, UINT slot)
	{
		return Codex::Resolve<Texture>(gfx, path, slot);
	}

	inline std::string Texture::GenerateRID(const std::string & path, UINT slot) noexcept
	{
		return "#" + std::string(typeid(Texture).name()) + "#" + path + "#" + std::to_string(slot) + "#";
	}
}
