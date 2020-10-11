#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class Rasterizer : public IBindable
	{
		D3D11_CULL_MODE culling;
		bool depthEnable;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> state;

	public:
		Rasterizer(Graphics& gfx, D3D11_CULL_MODE culling, bool depthEnable = true);
		virtual ~Rasterizer() = default;

		static inline std::shared_ptr<Rasterizer> Get(Graphics& gfx, D3D11_CULL_MODE culling, bool depthEnable = true) { return Codex::Resolve<Rasterizer>(gfx, culling, depthEnable); }
		static inline std::string GenerateRID(D3D11_CULL_MODE culling, bool depthEnable = true) noexcept;

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->RSSetState(state.Get()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(culling, depthEnable); }
	};

	template<>
	struct is_resolvable_by_codex<Rasterizer>
	{
		static constexpr bool generate{ true };
	};

	inline std::string Rasterizer::GenerateRID(D3D11_CULL_MODE culling, bool depthEnable) noexcept
	{
		return "#" + std::string(typeid(Rasterizer).name()) + "#" + std::to_string(culling) + "#" + std::to_string(depthEnable) + "#";
	}
}