#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class Texture : public IBindable
	{
		UINT slot;
		bool alpha = false;
		std::string path;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

	public:
		Texture(Graphics& gfx, const std::string& path, UINT slot = 0U, bool alphaEnable = false);
		virtual ~Texture() = default;

		static inline std::shared_ptr<Texture> Get(Graphics& gfx, const std::string& path, UINT slot = 0U, bool alphaEnable = false);
		static inline std::string GenerateRID(const std::string& path, UINT slot = 0U, bool alphaEnable = false) noexcept;

		constexpr bool HasAlpha() const noexcept { return alpha; }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(path, slot); }
	};

	template<>
	struct is_resolvable_by_codex<Texture>
	{
		static constexpr bool generate{ true };
	};

	inline std::shared_ptr<Texture> Texture::Get(Graphics& gfx, const std::string& path, UINT slot, bool alphaEnable)
	{
		return Codex::Resolve<Texture>(gfx, path, slot, alphaEnable);
	}

	inline std::string Texture::GenerateRID(const std::string& path, UINT slot, bool alphaEnable) noexcept
	{
		return "#" + std::string(typeid(Texture).name()) + "#" + path + "#" + std::to_string(slot) + "#";
	}
}