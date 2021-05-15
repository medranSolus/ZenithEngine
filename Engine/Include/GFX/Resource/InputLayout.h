#pragma once
#include "VertexShader.h"
#include "GFX/Data/VertexLayout.h"

namespace ZE::GFX::Resource
{
	class InputLayout : public IBindable
	{
		std::shared_ptr<Data::VertexLayout> vertexLayout = nullptr;
		GfxResPtr<VertexShader> shader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	public:
		InputLayout(Graphics& gfx, std::shared_ptr<Data::VertexLayout> layout, const GfxResPtr<VertexShader>& shader);
		virtual ~InputLayout() = default;

		static std::string GenerateRID(std::shared_ptr<Data::VertexLayout> vertexLayout, const GfxResPtr<VertexShader>& shader) noexcept;
		static GfxResPtr<InputLayout> Get(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout, const GfxResPtr<VertexShader>& shader);

		void Bind(Graphics& gfx) const override { GetContext(gfx)->IASetInputLayout(inputLayout.Get()); }
		std::string GetRID() const noexcept override { return GenerateRID(vertexLayout, shader); }
	};

	template<>
	struct is_resolvable_by_codex<InputLayout>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}