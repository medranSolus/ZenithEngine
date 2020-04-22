#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class DepthStencil : public IBindable
	{
	public:
		enum StencilMode : unsigned char { Off, Write, Mask };

	private:
		StencilMode mode;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state;

	public:
		DepthStencil(Graphics& gfx, StencilMode mode);
		DepthStencil(const DepthStencil&) = delete;
		DepthStencil& operator=(const DepthStencil&) = delete;
		virtual ~DepthStencil() = default;

		static inline std::shared_ptr<DepthStencil> Get(Graphics& gfx, StencilMode mode) { return Codex::Resolve<DepthStencil>(gfx, mode); }
		static inline std::string GenerateRID(StencilMode mode) noexcept { return "#" + std::string(typeid(DepthStencil).name()) + "#" + std::to_string(mode) + "#"; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->OMSetDepthStencilState(state.Get(), 0xFF); }
		inline std::string GetRID() const noexcept override { return GenerateRID(mode); }
	};

	template<>
	struct is_resolvable_by_codex<DepthStencil>
	{
		static constexpr bool generate{ true };
	};
}