#pragma once
#include "GfxResPtr.h"

namespace ZE::GFX::Resource
{
	class Blender : public IBindable
	{
	public:
		enum Type : U8 { None, Light, Normal };

	private:
		Type type;
		Microsoft::WRL::ComPtr<ID3D11BlendState> state;

	public:
		Blender(Graphics& gfx, Type type);
		virtual ~Blender() = default;

		static std::string GenerateRID(Type type) noexcept { return "B" + std::to_string(type); }
		static ResPtr<Blender> Get(Graphics& gfx, Type type) { return Codex::Resolve<Blender>(gfx, type); }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->OMSetBlendState(state.Get(), nullptr, 0xFFFFFFFF); }
		std::string GetRID() const noexcept override { return GenerateRID(type); }
	};

	template<>
	struct is_resolvable_by_codex<Blender>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}