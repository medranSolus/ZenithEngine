#pragma once
#include "GfxResPtr.h"

namespace ZE::GFX::Resource
{
	class DepthStencilState : public IBindable
	{
	public:
		enum StencilMode : U8 { Off, Write, Mask, DepthOff, Reverse, DepthFirst };

	private:
		StencilMode mode;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state;

	public:
		DepthStencilState(Graphics& gfx, StencilMode mode);
		virtual ~DepthStencilState() = default;

		static std::string GenerateRID(StencilMode mode) noexcept { return "S" + std::to_string(mode); }
		static GfxResPtr<DepthStencilState> Get(Graphics& gfx, StencilMode mode) { return Codex::Resolve<DepthStencilState>(gfx, mode); }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->OMSetDepthStencilState(state.Get(), 0xFF); }
		std::string GetRID() const noexcept override { return GenerateRID(mode); }
	};

	template<>
	struct is_resolvable_by_codex<DepthStencilState>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}