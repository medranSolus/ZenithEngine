#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class Blender : public IBindable
	{
	public:
		enum Type : uint8_t { None, Light, Normal };

	private:
		Type type;
		Microsoft::WRL::ComPtr<ID3D11BlendState> state;

	public:
		Blender(Graphics& gfx, Type type);
		virtual ~Blender() = default;

		static inline ResPtr<Blender> Get(Graphics& gfx, Type type) { return Codex::Resolve<Blender>(gfx, type); }
		static inline std::string GenerateRID(Type type) noexcept { return "B" + std::to_string(type); }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->OMSetBlendState(state.Get(), nullptr, 0xFFFFFFFFU); }
		inline std::string GetRID() const noexcept override { return GenerateRID(type); }
	};

	template<>
	struct is_resolvable_by_codex<Blender>
	{
		static constexpr bool generate{ true };
	};
}