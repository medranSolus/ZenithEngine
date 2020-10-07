#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class GeometryShader : public IBindable
	{
		std::string path;
		Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader;

	public:
		GeometryShader(Graphics& gfx, const std::string& path);
		virtual ~GeometryShader() = default;

		static inline std::shared_ptr<GeometryShader> Get(Graphics& gfx, const std::string& path) { return Codex::Resolve<GeometryShader>(gfx, path); }
		static inline std::string GenerateRID(const std::string& path) noexcept { return "#" + std::string(typeid(GeometryShader).name()) + "#" + path + "#"; }

		constexpr const std::string& GetName() const noexcept { return path; }
		inline ID3DBlob* GetBytecode() const noexcept { return bytecode.Get(); }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->GSSetShader(geometryShader.Get(), nullptr, 0U); }
		inline std::string GetRID() const noexcept override { return GenerateRID(path); }
	};

	template<>
	struct is_resolvable_by_codex<GeometryShader>
	{
		static constexpr bool generate{ true };
	};
}