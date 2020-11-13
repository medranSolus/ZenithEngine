#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class VertexShader : public IBindable
	{
		std::string name;
		Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;

	public:
		VertexShader(Graphics& gfx, const std::string& name);
		virtual ~VertexShader() = default;

		static inline GfxResPtr<VertexShader> Get(Graphics& gfx, const std::string& name) { return Codex::Resolve<VertexShader>(gfx, name); }
		static inline std::string GenerateRID(const std::string& name) noexcept { return "VS#" + name; }

		constexpr const std::string& GetName() const noexcept { return name; }
		inline ID3DBlob* GetBytecode() const noexcept { return bytecode.Get(); }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->VSSetShader(vertexShader.Get(), nullptr, 0U); }
		inline std::string GetRID() const noexcept override { return GenerateRID(name); }
	};

	template<>
	struct is_resolvable_by_codex<VertexShader>
	{
		static constexpr bool generate{ true };
	};
}