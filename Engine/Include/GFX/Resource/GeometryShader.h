#pragma once
#include "GfxResPtr.h"

namespace ZE::GFX::Resource
{
	class GeometryShader : public IBindable
	{
		std::string name;
		Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader;

	public:
		GeometryShader(Graphics& gfx, const std::string& name);
		virtual ~GeometryShader() = default;

		static std::string GenerateRID(const std::string& name) noexcept { return "G#" + name; }
		static GfxResPtr<GeometryShader> Get(Graphics& gfx, const std::string& name) { return Codex::Resolve<GeometryShader>(gfx, name); }

		constexpr const std::string& GetName() const noexcept { return name; }
		ID3DBlob* GetBytecode() const noexcept { return bytecode.Get(); }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->GSSetShader(geometryShader.Get(), nullptr, 0); }
		std::string GetRID() const noexcept override { return GenerateRID(name); }
	};

	template<>
	struct is_resolvable_by_codex<GeometryShader>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}