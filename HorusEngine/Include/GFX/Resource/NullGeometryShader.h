#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class NullGeometryShader : public IBindable
	{
	public:
		constexpr NullGeometryShader(Graphics& gfx) noexcept {}
		virtual ~NullGeometryShader() = default;

		static constexpr const char* GenerateRID() noexcept { return "G"; }
		static GfxResPtr<NullGeometryShader> Get(Graphics& gfx) noexcept { return Codex::Resolve<NullGeometryShader>(gfx); }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->GSSetShader(nullptr, nullptr, 0); }
		std::string GetRID() const noexcept override { return GenerateRID(); }
	};

	template<>
	struct is_resolvable_by_codex<NullGeometryShader>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}