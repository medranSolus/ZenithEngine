#pragma once
#include "VertexShader.h"
#include "VertexLayout.h"

namespace GFX::Resource
{
	class InputLayout : public IBindable
	{
		std::shared_ptr<Data::VertexLayout> vertexLayout = nullptr;
		std::shared_ptr<VertexShader> shader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	public:
		InputLayout(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout, std::shared_ptr<VertexShader> shader);
		virtual ~InputLayout() = default;

		static inline std::shared_ptr<InputLayout> Get(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout, std::shared_ptr<VertexShader> shader);
		static inline std::string GenerateRID(std::shared_ptr<Data::VertexLayout> vertexLayout, std::shared_ptr<VertexShader> shader) noexcept;

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->IASetInputLayout(inputLayout.Get()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(vertexLayout, shader); }
	};

	template<>
	struct is_resolvable_by_codex<InputLayout>
	{
		static constexpr bool generate{ true };
	};

	inline std::shared_ptr<InputLayout> InputLayout::Get(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout, std::shared_ptr<VertexShader> shader)
	{
		return Codex::Resolve<InputLayout>(gfx, vertexLayout, shader);
	}

	inline std::string InputLayout::GenerateRID(std::shared_ptr<Data::VertexLayout> vertexLayout, std::shared_ptr<VertexShader> shader) noexcept
	{
		return "#" + std::string(typeid(InputLayout).name()) + "#" + vertexLayout->GetLayoutCode() + "#" + shader->GetName() + "#";
	}
}