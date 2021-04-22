#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class TextureCube : public IBindable
	{
		U32 slot;
		std::string path;
		std::string ext;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

		void SetTexture(Graphics& gfx, const std::string& dir, const std::string& fileExt);

	public:
		TextureCube(Graphics& gfx, const std::string& path, const std::string& ext, U32 slot = 0) : slot(slot) { SetTexture(gfx, path, ext); }
		virtual ~TextureCube() = default;

		static std::string GenerateRID(const std::string& path, const std::string& ext, U32 slot = 0) noexcept;
		static GfxResPtr<TextureCube> Get(Graphics& gfx, const std::string& path, const std::string& ext, U32 slot = 0);

		void ChangeFile(Graphics& gfx, const std::string& path, const std::string& ext) { SetTexture(gfx, path, ext); }
		void Bind(Graphics& gfx) const override { GetContext(gfx)->PSSetShaderResources(slot, 1, textureView.GetAddressOf()); }
		std::string GetRID() const noexcept override { return GenerateRID(path, ext, slot); }
	};

	template<>
	struct is_resolvable_by_codex<TextureCube>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}