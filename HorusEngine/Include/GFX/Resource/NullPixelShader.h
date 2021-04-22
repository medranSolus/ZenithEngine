#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class NullPixelShader : public IBindable
	{
	public:
		constexpr NullPixelShader(Graphics& gfx) noexcept {}
		virtual ~NullPixelShader() = default;

		static constexpr const char* GenerateRID() noexcept { return "P"; }
		static GfxResPtr<NullPixelShader> Get(Graphics& gfx) noexcept { return Codex::Resolve<NullPixelShader>(gfx); }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->PSSetShader(nullptr, nullptr, 0); }
		std::string GetRID() const noexcept override { return GenerateRID(); }
	};

	template<>
	struct is_resolvable_by_codex<NullPixelShader>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}