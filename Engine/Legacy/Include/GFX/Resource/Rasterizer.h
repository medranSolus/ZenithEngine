#pragma once
#include "GfxResPtr.h"

namespace ZE::GFX::Resource
{
	class Rasterizer : public IBindable
	{
		D3D11_CULL_MODE culling;
		bool depthEnable;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> state;

	public:
		Rasterizer(Graphics& gfx, D3D11_CULL_MODE culling, bool depthEnable = true);
		virtual ~Rasterizer() = default;

		static std::string GenerateRID(D3D11_CULL_MODE culling, bool depthEnable = true) noexcept;
		static GfxResPtr<Rasterizer> Get(Graphics& gfx, D3D11_CULL_MODE culling, bool depthEnable = true);

		void Bind(Graphics& gfx) const override { GetContext(gfx)->RSSetState(state.Get()); }
		std::string GetRID() const noexcept override { return GenerateRID(culling, depthEnable); }
	};

	template<>
	struct is_resolvable_by_codex<Rasterizer>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}