#pragma once
#include "GfxResPtr.h"
#include "Surface.h"

namespace GFX::Resource
{
	class Texture : public IBindable
	{
		UINT slot;
		bool alpha = false;
		std::string path;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

	public:
		inline Texture(Graphics& gfx, const std::string& path, UINT slot = 0U, bool alphaEnable = false) :
			Texture(gfx, Surface(path), path, slot, alphaEnable) {}
		Texture(Graphics& gfx, const Surface& surface, const std::string& name, UINT slot = 0U, bool alphaEnable = false);
		virtual ~Texture() = default;

		static inline GfxResPtr<Texture> Get(Graphics& gfx, const std::string& path, UINT slot = 0U, bool alphaEnable = false);
		static inline GfxResPtr<Texture> Get(Graphics& gfx, const Surface& surface, const std::string& name, UINT slot = 0U, bool alphaEnable = false);
		static inline std::string GenerateRID(const std::string& path, UINT slot = 0U, bool alphaEnable = false) noexcept;
		static inline std::string GenerateRID(const Surface& surface, const std::string& name, UINT slot = 0U, bool alphaEnable = false) noexcept;

		constexpr bool HasAlpha() const noexcept { return alpha; }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(path, slot); }
	};

	template<>
	struct is_resolvable_by_codex<Texture>
	{
		static constexpr bool generate{ true };
	};

	inline GfxResPtr<Texture> Texture::Get(Graphics& gfx, const std::string& path, UINT slot, bool alphaEnable)
	{
		return Codex::Resolve<Texture>(gfx, Surface(path), path, slot, alphaEnable);
	}

	inline GfxResPtr<Texture> Texture::Get(Graphics& gfx, const Surface& surface, const std::string& name, UINT slot, bool alphaEnable)
	{
		return Codex::Resolve<Texture>(gfx, surface, name, slot, alphaEnable);
	}

	inline std::string Texture::GenerateRID(const std::string& path, UINT slot, bool alphaEnable) noexcept
	{
		return "TX" + std::to_string(slot) + "#" + path;
	}

	inline std::string Texture::GenerateRID(const Surface& surface, const std::string& name, UINT slot, bool alphaEnable) noexcept
	{
		return GenerateRID(name, slot, alphaEnable);
	}
}