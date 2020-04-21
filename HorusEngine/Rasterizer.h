#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class Rasterizer : public IBindable
	{
		bool culling;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> state;

	public:
		Rasterizer(Graphics& gfx, bool culling);
		Rasterizer(const Rasterizer&) = delete;
		Rasterizer& operator=(const Rasterizer&) = delete;
		virtual ~Rasterizer() = default;

		static inline std::shared_ptr<Rasterizer> Get(Graphics& gfx, bool culling) { return Codex::Resolve<Rasterizer>(gfx, culling); }
		static inline std::string GenerateRID(bool culling) noexcept { return "#" + std::string(typeid(Rasterizer).name()) + "#" + std::to_string(culling) + "#"; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->RSSetState(state.Get()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(culling); }
	};

	template<>
	struct is_resolvable_by_codex<Rasterizer>
	{
		static constexpr bool value{ true };
	};
}