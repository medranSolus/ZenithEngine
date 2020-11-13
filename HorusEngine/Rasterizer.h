#pragma once
#include "GfxResPtr.h"

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

		static inline GfxResPtr<Rasterizer> Get(Graphics& gfx, D3D11_CULL_MODE culling, bool depthEnable = true) { return Codex::Resolve<Rasterizer>(gfx, culling, depthEnable); }
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
		return "R" + std::to_string(depthEnable) + "#" + std::to_string(culling);
	}
}