#pragma once
#include "Pixel.h"
#include "ColorF3.h"
#include "ColorF4.h"
#include "assimp/scene.h"
#include <d3d11.h>
#include <vector>
#include <string>

// Extract DataType from assimp mesh (removed reinterpret_cast for constexpr behavior)
#define ZE_VERTEX_ELEMENT_AI_EXTRACTOR(member) \
	static constexpr DataType Extract(const aiMesh& mesh, U64 i) noexcept \
	{ \
		return *(const DataType*)(&mesh.member[i]); \
	}

// List of vertex layout element types names. Each name have invocation of macro X() on it. Define your X() macro for various behavior in code.
#define ZE_VERTEX_LAYOUT_ELEMENTS \
	X(Position2D) \
	X(Position3D) \
	X(Texture2D) \
	X(Normal) \
	X(Bitangent) \
	X(ColorFloat4) \
	X(ColorFloat3) \
	X(ColorByte)

namespace ZE::GFX::Data
{
	// Managment type for VertexBufferData used to decode vertex structure
	class VertexLayout final
	{
	public:
		enum class ElementType : U8
		{
#define X(el) el,
			ZE_VERTEX_LAYOUT_ELEMENTS
#undef X
			Count,
		};

		// Type of single descriptor of layout
		template<ElementType>
		struct Desc { static constexpr bool VALID = false; };

		// Single element inside vertex buffer
		class Element final
		{
#pragma region Brigde functors
			template<VertexLayout::ElementType Type>
			struct CodeLookup
			{
				static constexpr const char* Exec() noexcept { return Desc<Type>::CODE; }
			};
			template<VertexLayout::ElementType Type>
			struct SizeLookup
			{
				static constexpr U64 Exec() noexcept { return sizeof(Desc<Type>::DataType); }
			};
			template<VertexLayout::ElementType Type>
			struct DescGenerate
			{
				static constexpr D3D11_INPUT_ELEMENT_DESC Exec(U64 offset) noexcept
				{
					return
					{
						Desc<Type>::SEMANTIC, 0, Desc<Type>::DATA_FORMAT,
						0, static_cast<UINT>(offset), D3D11_INPUT_PER_VERTEX_DATA, 0
					};
				}
			};
#pragma endregion

			ElementType type;
			U64 offset;

		public:
			constexpr Element(ElementType type, U64 offset) noexcept : type(type), offset(offset) {}
			Element(Element&&) = default;
			Element(const Element&) = default;
			Element& operator=(Element&&) = default;
			Element& operator=(const Element&) = default;
			~Element() = default;

			static constexpr U64 SizeOf(ElementType type) noexcept { return VertexLayout::Bridge<SizeLookup>(type); }

			constexpr ElementType GetType() const noexcept { return type; }
			constexpr U64 Size() const noexcept { return SizeOf(type); }
			constexpr U64 GetOffset() const noexcept { return offset; }
			constexpr U64 GetEnd() const noexcept { return offset + Size(); }

			constexpr const char* GetCode() const noexcept { return VertexLayout::Bridge<CodeLookup>(type); }
			constexpr D3D11_INPUT_ELEMENT_DESC GetDesc() const noexcept { return VertexLayout::Bridge<DescGenerate>(type, GetOffset()); }
		};

	private:
		std::vector<Element> elements;

	public:
		VertexLayout(bool position3D = true) noexcept;
		VertexLayout(VertexLayout&&) = default;
		VertexLayout(const VertexLayout&) = default;
		VertexLayout& operator=(VertexLayout&&) = default;
		VertexLayout& operator=(const VertexLayout&) = default;
		~VertexLayout() = default;

		template<template<ElementType> class Functor, typename... Params>
		static constexpr decltype(auto) Bridge(ElementType type, Params&&... p) noexcept
		{
			switch (type)
			{
#define X(el) \
			case ElementType::el: \
				return Functor<ElementType::el>::Exec(std::forward<Params>(p)...);
				ZE_VERTEX_LAYOUT_ELEMENTS
#undef X
			}
			assert("Invalid element type" && false);
			return Functor<ElementType::Count>::Exec(std::forward<Params>(p)...);
		}

		U64 GetElementCount() const noexcept { return elements.size(); }
		U64 Size() const noexcept { return elements.empty() ? 0 : elements.back().GetEnd(); }
		const Element& ResolveByIndex(U64 i) const { return elements.at(i); }

		bool Has(ElementType type) const noexcept;
		const Element& Resolve(ElementType type) const noexcept;
		VertexLayout& Append(ElementType type) noexcept;
		std::vector<D3D11_INPUT_ELEMENT_DESC> GetLayout() const noexcept;
		std::string GetLayoutCode() const noexcept;

#pragma region Layout Element Info
		template<> struct Desc<ElementType::Position2D>
		{
			using DataType = Float2;
			static constexpr DXGI_FORMAT DATA_FORMAT = DXGI_FORMAT_R32G32_FLOAT;
			static constexpr const char* SEMANTIC = "POSITION";
			static constexpr const char* CODE = "P2";
			static constexpr bool VALID = true;
			ZE_VERTEX_ELEMENT_AI_EXTRACTOR(mVertices)
		};
		template<> struct Desc<ElementType::Position3D>
		{
			using DataType = Float3;
			static constexpr DXGI_FORMAT DATA_FORMAT = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* SEMANTIC = "POSITION";
			static constexpr const char* CODE = "P";
			static constexpr bool VALID = true;
			ZE_VERTEX_ELEMENT_AI_EXTRACTOR(mVertices)
		};
		template<> struct Desc<ElementType::Texture2D>
		{
			using DataType = Float2;
			static constexpr DXGI_FORMAT DATA_FORMAT = DXGI_FORMAT_R32G32_FLOAT;
			static constexpr const char* SEMANTIC = "TEXCOORD";
			static constexpr const char* CODE = "T";
			static constexpr bool VALID = true;
			ZE_VERTEX_ELEMENT_AI_EXTRACTOR(mTextureCoords[0])
		};
		template<> struct Desc<ElementType::Normal>
		{
			using DataType = Float3;
			static constexpr DXGI_FORMAT DATA_FORMAT = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* SEMANTIC = "NORMAL";
			static constexpr const char* CODE = "N";
			static constexpr bool VALID = true;
			ZE_VERTEX_ELEMENT_AI_EXTRACTOR(mNormals)
		};
		template<> struct Desc<ElementType::Bitangent>
		{
			using DataType = Float3;
			static constexpr DXGI_FORMAT DATA_FORMAT = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* SEMANTIC = "BITANGENT";
			static constexpr const char* CODE = "B";
			static constexpr bool VALID = true;
			ZE_VERTEX_ELEMENT_AI_EXTRACTOR(mBitangents)
		};
		template<> struct Desc<ElementType::ColorFloat4>
		{
			using DataType = ColorF4;
			static constexpr DXGI_FORMAT DATA_FORMAT = DXGI_FORMAT_R32G32B32A32_FLOAT;
			static constexpr const char* SEMANTIC = "COLOR";
			static constexpr const char* CODE = "C";
			static constexpr bool VALID = true;
			ZE_VERTEX_ELEMENT_AI_EXTRACTOR(mColors[0])
		};
		template<> struct Desc<ElementType::ColorFloat3>
		{
			using DataType = ColorF3;
			static constexpr DXGI_FORMAT DATA_FORMAT = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* SEMANTIC = "COLOR";
			static constexpr const char* CODE = "C3";
			static constexpr bool VALID = true;
			ZE_VERTEX_ELEMENT_AI_EXTRACTOR(mColors[0])
		};
		template<> struct Desc<ElementType::ColorByte>
		{
			using DataType = Pixel;
			static constexpr DXGI_FORMAT DATA_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
			static constexpr const char* SEMANTIC = "COLOR";
			static constexpr const char* CODE = "CB";
			static constexpr bool VALID = true;
			ZE_VERTEX_ELEMENT_AI_EXTRACTOR(mColors[0])
		};
		template<> struct Desc<ElementType::Count>
		{
			using DataType = U8;
			static constexpr DXGI_FORMAT DATA_FORMAT = DXGI_FORMAT_UNKNOWN;
			static constexpr const char* SEMANTIC = "?";
			static constexpr const char* CODE = "?";
			ZE_VERTEX_ELEMENT_AI_EXTRACTOR(mFaces)
		};

		// Sanity check to make sure that all elements have corresponding Desc specialization.
#define X(el) static_assert(Desc<ElementType::el>::VALID, "Missing Desc implementation for " #el);
		ZE_VERTEX_LAYOUT_ELEMENTS
#undef X
#pragma endregion
	};
}

namespace ZE
{
	typedef GFX::Data::VertexLayout::ElementType VertexAttribute;
}

#undef ZE_VERTEX_LAYOUT_ELEMENTS
#undef ZE_VERTEX_ELEMENT_AI_EXTRACTOR