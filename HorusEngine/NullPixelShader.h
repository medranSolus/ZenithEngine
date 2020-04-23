#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class NullPixelShader : public IBindable
	{
	public:
		constexpr NullPixelShader(Graphics& gfx) noexcept {}
		NullPixelShader(const NullPixelShader&) = delete;
		NullPixelShader& operator=(const NullPixelShader&) = delete;
		virtual ~NullPixelShader() = default;

		static inline std::shared_ptr<NullPixelShader> Get(Graphics& gfx) { return Codex::Resolve<NullPixelShader>(gfx); }
		static inline std::string GenerateRID() noexcept { return "#" + std::string(typeid(NullPixelShader).name()) + "#"; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetShader(nullptr, nullptr, 0U); }
		inline std::string GetRID() const noexcept override { return GenerateRID(); }
	};

	template<>
	struct is_resolvable_by_codex<NullPixelShader>
	{
		static constexpr bool generate{ true };
	};
}