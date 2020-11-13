#pragma once
#include "VertexShader.h"
#include "VertexLayout.h"

namespace GFX::Resource
{
	class InputLayout : public IBindable
	{
		std::shared_ptr<Data::VertexLayout> vertexLayout = nullptr;
		GfxResPtr<VertexShader> shader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	public:
		InputLayout(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout, const GfxResPtr<VertexShader>& shader);
		virtual ~InputLayout() = default;

		static inline GfxResPtr<InputLayout> Get(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout, const GfxResPtr<VertexShader>& shader);
		static inline std::string GenerateRID(std::shared_ptr<Data::VertexLayout> vertexLayout, const GfxResPtr<VertexShader>& shader) noexcept;

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->IASetInputLayout(inputLayout.Get()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(vertexLayout, shader); }
	};

	template<>
	struct is_resolvable_by_codex<InputLayout>
	{
		static constexpr bool generate{ true };
	};

	inline GfxResPtr<InputLayout> InputLayout::Get(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout, const GfxResPtr<VertexShader>& shader)
	{
		return Codex::Resolve<InputLayout>(gfx, vertexLayout, shader);
	}

	inline std::string InputLayout::GenerateRID(std::shared_ptr<Data::VertexLayout> vertexLayout, const GfxResPtr<VertexShader>& shader) noexcept
	{
		return "IL" + vertexLayout->GetLayoutCode() + "#" + shader->GetName();
	}
}