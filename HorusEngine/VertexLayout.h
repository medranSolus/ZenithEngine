#pragma once
#include "Color.h"
#include <d3d11.h>
#include <vector>
#include <string>

namespace GFX::BasicType
{
	// Managment type for VertexDataBuffer used to decode vertex structure
	class VertexLayout
	{
	public:
		enum ElementType
		{
			Position3D,
			Texture2D,
			Normal,
			ColorFloat,
			ColorByte,
			Count,
		};

		// Type of single descriptor of layout
		template<ElementType> struct Desc;

		// Single element inside vertex buffer
		class Element
		{
			ElementType type;
			size_t offset;

			template<ElementType type>
			static constexpr D3D11_INPUT_ELEMENT_DESC GenerateDesc(size_t offset) noexcept
			{
				return { Desc<type>::semantic, 0, Desc<type>::dxgiFormat, 0, static_cast<UINT>(offset), D3D11_INPUT_PER_VERTEX_DATA, 0 };
			}

		public:
			constexpr Element(ElementType type, size_t offset) : type(type), offset(offset) {}
			Element& operator=(const Element&) = default;
			~Element() = default;

			constexpr size_t GetEnd() const noexcept(!IS_DEBUG) { return offset + Size(); }
			constexpr size_t GetOffset() const { return offset; }
			constexpr size_t Size() const noexcept(!IS_DEBUG) { return SizeOf(type); }
			constexpr ElementType GetType() const noexcept { return type; }

			static constexpr size_t SizeOf(ElementType type) noexcept(!IS_DEBUG);

			constexpr const char* GetCode() const noexcept;
			constexpr D3D11_INPUT_ELEMENT_DESC GetDesc() const noexcept(!IS_DEBUG);
		};

	private:
		std::vector<Element> elements;

	public:
		VertexLayout(bool position3D = true) noexcept;
		VertexLayout& operator=(const VertexLayout&) = default;
		~VertexLayout() = default;

		template<ElementType Type>
		const Element& Resolve() const noexcept(!IS_DEBUG);

		inline const Element& ResolveByIndex(size_t i) const { return elements.at(i); }
		inline size_t Size() const noexcept(!IS_DEBUG) { return elements.empty() ? 0U : elements.back().GetEnd(); }
		inline size_t GetElementCount() const noexcept { return elements.size(); }

		VertexLayout& Append(ElementType type) noexcept(!IS_DEBUG);
		std::vector<D3D11_INPUT_ELEMENT_DESC> GetDXLayout() const noexcept(!IS_DEBUG);
		std::string GetLayoutCode() const noexcept(!IS_DEBUG);

#pragma region LayoutTypes
		template<> struct Desc<Position3D>
		{
			using DataType = DirectX::XMFLOAT3;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* semantic = "POSITION";
			static constexpr const char* code = "P3";
		};

		template<> struct Desc<Texture2D>
		{
			using DataType = DirectX::XMFLOAT2;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
			static constexpr const char* semantic = "TEXCOORD";
			static constexpr const char* code = "T2";
		};

		template<> struct Desc<Normal>
		{
			using DataType = DirectX::XMFLOAT3;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* semantic = "NORMAL";
			static constexpr const char* code = "N";
		};

		template<> struct Desc<ColorFloat>
		{
			using DataType = GFX::BasicType::ColorFloat;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
			static constexpr const char* semantic = "COLOR";
			static constexpr const char* code = "C4";
		};

		template<> struct Desc<ColorByte>
		{
			using DataType = GFX::BasicType::ColorByte;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			static constexpr const char* semantic = "COLOR";
			static constexpr const char* code = "C1";
		};
#pragma endregion
	};

	constexpr size_t VertexLayout::Element::SizeOf(VertexLayout::ElementType type) noexcept(!IS_DEBUG)
	{
		switch (type)
		{
		case Position3D:
			return sizeof(Desc<Position3D>::DataType);
		case Texture2D:
			return sizeof(Desc<Texture2D>::DataType);
		case Normal:
			return sizeof(Desc<Normal>::DataType);
		case ColorFloat:
			return sizeof(Desc<ColorFloat>::DataType);
		case ColorByte:
			return sizeof(Desc<ColorByte>::DataType);
		}
		assert("Invalid element type" && false);
		return 0U;
	}

	template<VertexLayout::ElementType Type>
	const VertexLayout::Element& VertexLayout::Resolve() const noexcept(!IS_DEBUG)
	{
		for (auto& e : elements)
			if (e.GetType() == Type)
				return e;
		assert("Could not resolve element type" && false);
		return elements.front();
	}
}

typedef GFX::BasicType::VertexLayout::ElementType VertexAttribute;
