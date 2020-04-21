#pragma once
#include "Codex.h"
#include "VertexLayout.h"

namespace GFX::Resource
{
	class InputLayout : public IBindable
	{
	protected:
		std::shared_ptr<Data::VertexLayout> vertexLayout = nullptr;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	public:
		InputLayout(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout, ID3DBlob* vertexShaderBytecode);
		InputLayout(const InputLayout&) = delete;
		InputLayout& operator=(const InputLayout&) = delete;
		virtual ~InputLayout() = default;

		static inline std::shared_ptr<InputLayout> Get(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout, ID3DBlob* vertexShaderBytecode);
		static inline std::string GenerateRID(std::shared_ptr<Data::VertexLayout> vertexLayout, ID3DBlob* vertexShaderBytecode = nullptr) noexcept;

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->IASetInputLayout(inputLayout.Get()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(vertexLayout); }
	};

	template<>
	struct is_resolvable_by_codex<InputLayout>
	{
		static constexpr bool value{ true };
	};

	inline std::shared_ptr<InputLayout> InputLayout::Get(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout, ID3DBlob* vertexShaderBytecode)
	{
		return Codex::Resolve<InputLayout>(gfx, vertexLayout, vertexShaderBytecode);
	}

	inline std::string InputLayout::GenerateRID(std::shared_ptr<Data::VertexLayout> vertexLayout, ID3DBlob* vertexShaderBytecode) noexcept
	{
		return "#" + std::string(typeid(InputLayout).name()) + "#" + vertexLayout->GetLayoutCode() + "#";
	}
}