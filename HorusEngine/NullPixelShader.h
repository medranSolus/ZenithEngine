#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class NullPixelShader : public IBindable
	{
	public:
		constexpr NullPixelShader(Graphics& gfx) noexcept {}
		virtual ~NullPixelShader() = default;

		static inline GfxResPtr<NullPixelShader> Get(Graphics& gfx) noexcept { return Codex::Resolve<NullPixelShader>(gfx); }
		static inline std::string GenerateRID() noexcept { return "NP"; }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetShader(nullptr, nullptr, 0U); }
		inline std::string GetRID() const noexcept override { return GenerateRID(); }
	};

	template<>
	struct is_resolvable_by_codex<NullPixelShader>
	{
		static constexpr bool generate{ true };
	};
}