#pragma once
#include "GfxResPtr.h"
#include "GFX/Surface.h"

namespace ZE::GFX::Resource
{
	class Texture : public IBindable
	{
		U32 slot;
		bool alpha = false;
		std::string path;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

	public:
		Texture(Graphics& gfx, const std::string& path, U32 slot = 0, bool alphaEnable = false) :
			Texture(gfx, Surface(path), path, slot, alphaEnable) {}
		Texture(Graphics& gfx, const Surface& surface, const std::string& name, U32 slot = 0, bool alphaEnable = false);
		virtual ~Texture() = default;

		static std::string GenerateRID(const std::string& path, U32 slot = 0, bool alphaEnable = false) noexcept;
		static std::string GenerateRID(const Surface& surface, const std::string& name, U32 slot = 0, bool alphaEnable = false) noexcept;
		static GfxResPtr<Texture> Get(Graphics& gfx, const std::string& path, U32 slot = 0, bool alphaEnable = false);
		static GfxResPtr<Texture> Get(Graphics& gfx, const Surface& surface, const std::string& name, U32 slot = 0, bool alphaEnable = false);

		constexpr bool HasAlpha() const noexcept { return alpha; }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->PSSetShaderResources(slot, 1, textureView.GetAddressOf()); }
		std::string GetRID() const noexcept override { return GenerateRID(path, slot); }
	};

	template<>
	struct is_resolvable_by_codex<Texture>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}