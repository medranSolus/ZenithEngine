#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class Blender : public IBindable
	{
		bool enabled;
		Microsoft::WRL::ComPtr<ID3D11BlendState> state;

	public:
		Blender(Graphics& gfx, bool enabled);
		Blender(const Blender&) = delete;
		Blender& operator=(const Blender&) = delete;
		~Blender() = default;

		static inline std::shared_ptr<Blender> Get(Graphics& gfx, bool enabled) { return Codex::Resolve<Blender>(gfx, enabled); }
		static inline std::string GenerateRID(bool enabled) noexcept { return "#" + std::string(typeid(Blender).name()) + "#" + std::to_string(enabled) + "#"; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->OMSetBlendState(state.Get(), nullptr, 0xFFFFFFFFU); }
		inline std::string GetRID() const noexcept override { return GenerateRID(enabled); }
	};

	template<>
	struct is_resolvable_by_codex<Blender>
	{
		static constexpr bool value{ true };
	};
}