#include "GFX/Resource/InputLayout.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace ZE::GFX::Resource
{
	InputLayout::InputLayout(Graphics& gfx, std::shared_ptr<Data::VertexLayout> layout, const GfxResPtr<VertexShader>& shader)
		: vertexLayout(std::move(layout)), shader(shader)
	{
		ZE_GFX_ENABLE_ALL(gfx);
		std::vector<D3D11_INPUT_ELEMENT_DESC> desc = vertexLayout->GetLayout();
		ID3DBlob* vertexShaderBytecode = shader->GetBytecode();
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateInputLayout(desc.data(), static_cast<UINT>(desc.size()),
			vertexShaderBytecode->GetBufferPointer(), vertexShaderBytecode->GetBufferSize(), &inputLayout));
		ZE_GFX_SET_RID(inputLayout.Get());
	}

	GfxResPtr<InputLayout> InputLayout::Get(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout, const GfxResPtr<VertexShader>& shader)
	{
		return Codex::Resolve<InputLayout>(gfx, vertexLayout, shader);
	}

	std::string InputLayout::GenerateRID(std::shared_ptr<Data::VertexLayout> vertexLayout, const GfxResPtr<VertexShader>& shader) noexcept
	{
		return "I" + vertexLayout->GetLayoutCode() + "#" + shader->GetName();
	}
}