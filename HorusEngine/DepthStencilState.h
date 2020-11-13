#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class DepthStencilState : public IBindable
	{
	public:
		enum StencilMode : uint8_t { Off, Write, Mask, DepthOff, Reverse, DepthFirst };

	private:
		StencilMode mode;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state;

	public:
		DepthStencilState(Graphics& gfx, StencilMode mode);
		virtual ~DepthStencilState() = default;

		static inline GfxResPtr<DepthStencilState> Get(Graphics& gfx, StencilMode mode) { return Codex::Resolve<DepthStencilState>(gfx, mode); }
		static inline std::string GenerateRID(StencilMode mode) noexcept { return "DSS" + std::to_string(mode); }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->OMSetDepthStencilState(state.Get(), 0xFF); }
		inline std::string GetRID() const noexcept override { return GenerateRID(mode); }
	};

	template<>
	struct is_resolvable_by_codex<DepthStencilState>
	{
		static constexpr bool generate{ true };
	};
}