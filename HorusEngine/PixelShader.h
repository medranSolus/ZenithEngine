#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class PixelShader : public IBindable
	{
	protected:
		std::string path;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

	public:
		PixelShader(Graphics& gfx, const std::string& path);
		PixelShader(const PixelShader&) = delete;
		PixelShader& operator=(const PixelShader&) = delete;
		~PixelShader() = default;

		static inline std::shared_ptr<PixelShader> Get(Graphics& gfx, const std::string& path) { return Codex::Resolve<PixelShader>(gfx, path); }
		static inline std::string GenerateRID(const std::string& path) noexcept { return "#" + std::string(typeid(PixelShader).name()) + "#" + path + "#"; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetShader(pixelShader.Get(), nullptr, 0U); }
		inline std::string GetRID() const noexcept override { return GenerateRID(path); }
	};

	template<>
	struct is_resolvable_by_codex<PixelShader>
	{
		static constexpr bool value{ true };
	};
}
