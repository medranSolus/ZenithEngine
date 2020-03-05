#pragma once
#include "Color.h"
#include <d3d11.h>
#include <vector>
#include <string>

// List of vertex layout element types names. Each name have invocation of macro X() on it. Define your X() macro for various behavior in code.
#define VERTEX_LAYOUT_ELEMENTS \
	X(Position3D) \
	X(Texture2D) \
	X(Normal) \
	X(Tangent) \
	X(Bitangent) \
	X(ColorFloat4) \
	X(ColorByte)

namespace GFX::Data
{
	// Managment type for VertexBufferData used to decode vertex structure
	class VertexLayout
	{
	public:
		enum class ElementType
		{
#define X(el) el,
			VERTEX_LAYOUT_ELEMENTS
#undef X
			Count,
		};

		// Type of single descriptor of layout
		template<ElementType>
		struct Desc
		{
			static constexpr bool valid = false;
		};

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

#pragma region Layout Element Info
		template<> struct Desc<ElementType::Position3D>
		{
			using DataType = DirectX::XMFLOAT3;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* semantic = "POSITION";
			static constexpr const char* code = "P3";
			static constexpr bool valid = true;
		};
		template<> struct Desc<ElementType::Texture2D>
		{
			using DataType = DirectX::XMFLOAT2;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
			static constexpr const char* semantic = "TEXCOORD";
			static constexpr const char* code = "T2";
			static constexpr bool valid = true;
		};
		template<> struct Desc<ElementType::Normal>
		{
			using DataType = DirectX::XMFLOAT3;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* semantic = "NORMAL";
			static constexpr const char* code = "N";
			static constexpr bool valid = true;
		};
		template<> struct Desc<ElementType::Tangent>
		{
			using DataType = DirectX::XMFLOAT3;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* semantic = "TANGENT";
			static constexpr const char* code = "T";
			static constexpr bool valid = true;
		};
		template<> struct Desc<ElementType::Bitangent>
		{
			using DataType = DirectX::XMFLOAT3;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* semantic = "BITANGENT";
			static constexpr const char* code = "B";
			static constexpr bool valid = true;
		};
		template<> struct Desc<ElementType::ColorFloat4>
		{
			using DataType = GFX::Data::ColorFloat4;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
			static constexpr const char* semantic = "COLOR";
			static constexpr const char* code = "C4";
			static constexpr bool valid = true;
		};
		template<> struct Desc<ElementType::ColorByte>
		{
			using DataType = GFX::Data::ColorByte;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			static constexpr const char* semantic = "COLOR";
			static constexpr const char* code = "C1";
			static constexpr bool valid = true;
		};

		// Sanity check to make sure that all elements have corresponding Desc specialization.
#define X(el) static_assert(Desc<ElementType::el>::valid, "Missing Desc implementation for " #el);
		VERTEX_LAYOUT_ELEMENTS
#undef X
#pragma endregion
	};

	constexpr size_t VertexLayout::Element::SizeOf(VertexLayout::ElementType type) noexcept(!IS_DEBUG)
	{
		switch (type)
		{
#define X(el) \
		case ElementType::el: \
			return sizeof(Desc<ElementType::el>::DataType);
			VERTEX_LAYOUT_ELEMENTS
#undef X
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

typedef GFX::Data::VertexLayout::ElementType VertexAttribute;

#ifndef VERTEX_LAYOUT_IMPL
#undef VERTEX_LAYOUT_ELEMENTS
#endif