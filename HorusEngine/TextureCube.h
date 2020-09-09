#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class TextureCube : public IBindable
	{
		UINT slot;
		std::string path;
		std::string ext;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

	public:
		TextureCube(Graphics& gfx, const std::string& path, const std::string& ext, UINT slot = 0U);
		virtual ~TextureCube() = default;

		static inline std::shared_ptr<TextureCube> Get(Graphics& gfx, const std::string& path, const std::string& ext, UINT slot = 0U);
		static inline std::string GenerateRID(const std::string& path, const std::string& ext, UINT slot = 0U) noexcept;

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetShaderResources(slot, 1U, textureView.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(path, ext, slot); }
	};

	template<>
	struct is_resolvable_by_codex<TextureCube>
	{
		static constexpr bool generate{ true };
	};

	inline std::shared_ptr<TextureCube> TextureCube::Get(Graphics& gfx, const std::string& path, const std::string& ext, UINT slot)
	{
		return Codex::Resolve<TextureCube>(gfx, path, ext, slot);
	}

	inline std::string TextureCube::GenerateRID(const std::string& path, const std::string& ext, UINT slot) noexcept
	{
		return "#" + std::string(typeid(TextureCube).name()) + "#" + path + ext + "#" + std::to_string(slot) + "#";
	}
}