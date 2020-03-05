#define VERTEX_LAYOUT_IMPL
#include "VertexLayout.h"

namespace GFX::Data
{
	constexpr const char* VertexLayout::Element::GetCode() const noexcept
	{
		switch (type)
		{
#define X(el) \
		case ElementType::el: \
			return Desc<ElementType::el>::code;
			VERTEX_LAYOUT_ELEMENTS
#undef X
		}
		assert("Invalid element type" && false);
		return "Invalid";
	}

	constexpr D3D11_INPUT_ELEMENT_DESC VertexLayout::Element::GetDesc() const noexcept(!IS_DEBUG)
	{
		switch (type)
		{
#define X(el) \
		case ElementType::el: \
			return GenerateDesc<ElementType::el>(GetOffset());
			VERTEX_LAYOUT_ELEMENTS
#undef X
		}
		assert("Invalid element type" && false);
		return { "INVALID", 0, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	}

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