#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class NullGeometryShader : public IBindable
	{
	public:
		constexpr NullGeometryShader(Graphics& gfx) noexcept {}
		virtual ~NullGeometryShader() = default;

		static inline GfxResPtr<NullGeometryShader> Get(Graphics& gfx) noexcept { return Codex::Resolve<NullGeometryShader>(gfx); }
		static inline std::string GenerateRID() noexcept { return "NG"; }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->GSSetShader(nullptr, nullptr, 0U); }
		inline std::string GetRID() const noexcept override { return GenerateRID(); }
	};

	template<>
	struct is_resolvable_by_codex<NullGeometryShader>
	{
		static constexpr bool generate{ true };
	};
}