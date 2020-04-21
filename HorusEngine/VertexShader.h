#pragma once
#include "Codex.h"

namespace GFX::Resource
{
	class VertexShader : public IBindable
	{
	protected:
		std::string path;
		Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;

	public:
		VertexShader(Graphics& gfx, const std::string& path);
		VertexShader(const VertexShader&) = delete;
		VertexShader& operator=(const VertexShader&) = delete;
		virtual ~VertexShader() = default;

		static inline std::shared_ptr<VertexShader> Get(Graphics& gfx, const std::string& path) { return Codex::Resolve<VertexShader>(gfx, path); }
		static inline std::string GenerateRID(const std::string& path) noexcept { return "#" + std::string(typeid(VertexShader).name()) + "#" + path + "#"; }

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->VSSetShader(vertexShader.Get(), nullptr, 0U); }
		inline std::string GetRID() const noexcept override { return GenerateRID(path); }

		inline ID3DBlob* GetBytecode() const noexcept { return bytecode.Get(); }
	};

	template<>
	struct is_resolvable_by_codex<VertexShader>
	{
		static constexpr bool value{ true };
	};
}