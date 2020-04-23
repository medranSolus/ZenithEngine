#define VERTEX_LAYOUT_IMPL
#include "VertexLayout.h"

namespace GFX::Data
{
	VertexLayout::VertexLayout(bool position3D) noexcept
	{
		if (position3D)
			elements.emplace_back(ElementType::Position3D, 0);
	}

	VertexLayout& VertexLayout::Append(ElementType type) noexcept(!IS_DEBUG)
	{
		elements.emplace_back(type, Size());
		return *this;
	}

	std::vector<D3D11_INPUT_ELEMENT_DESC> VertexLayout::GetDXLayout() const noexcept(!IS_DEBUG)
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
		desc.reserve(GetElementCount());
		for (const auto& e : elements)
			desc.emplace_back(e.GetDesc());
		return desc;
	}

	std::string VertexLayout::GetLayoutCode() const noexcept(!IS_DEBUG)
	{
		std::string code;
		for (const auto& e : elements)
			code += e.GetCode();
		return code;
	}
}