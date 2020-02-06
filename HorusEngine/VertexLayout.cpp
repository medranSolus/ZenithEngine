#include "VertexLayout.h"

namespace GFX::BasicType
{
	constexpr const char* VertexLayout::Element::GetCode() const noexcept
	{
		switch (type)
		{
		case Position3D:
			return Desc<Position3D>::code;
		case Texture2D:
			return Desc<Texture2D>::code;
		case Normal:
			return Desc<Normal>::code;
		case Tangent:
			return Desc<Tangent>::code;
		case Bitangent:
			return Desc<Bitangent>::code;
		case ColorFloat:
			return Desc<ColorFloat>::code;
		case ColorByte:
			return Desc<ColorByte>::code;
		}
		assert("Invalid element type" && false);
		return "Invalid";
	}

	constexpr D3D11_INPUT_ELEMENT_DESC VertexLayout::Element::GetDesc() const noexcept(!IS_DEBUG)
	{
		switch (type)
		{
		case Position3D:
			return GenerateDesc<Position3D>(GetOffset());
		case Texture2D:
			return GenerateDesc<Texture2D>(GetOffset());
		case Normal:
			return GenerateDesc<Normal>(GetOffset());
		case Tangent:
			return GenerateDesc<Tangent>(GetOffset());
		case Bitangent:
			return GenerateDesc<Bitangent>(GetOffset());
		case ColorFloat:
			return GenerateDesc<ColorFloat>(GetOffset());
		case ColorByte:
			return GenerateDesc<ColorByte>(GetOffset());
		}
		assert("Invalid element type" && false);
		return { "INVALID", 0, DXGI_FORMAT_UNKNOWN, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
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