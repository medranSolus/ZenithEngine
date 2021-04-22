#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class PixelShader : public IBindable
	{
		std::string name;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

	public:
		PixelShader(Graphics& gfx, const std::string& name);
		virtual ~PixelShader() = default;

		static std::string GenerateRID(const std::string& name) noexcept { return "P#" + name; }
		static GfxResPtr<PixelShader> Get(Graphics& gfx, const std::string& name) { return Codex::Resolve<PixelShader>(gfx, name); }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->PSSetShader(pixelShader.Get(), nullptr, 0); }
		std::string GetRID() const noexcept override { return GenerateRID(name); }
	};

	template<>
	struct is_resolvable_by_codex<PixelShader>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}