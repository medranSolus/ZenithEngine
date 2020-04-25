#pragma once
#include "Color.h"
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <cassert>

// List of leaf element types names. Each name have invocation of macro X() on it. Define your X() macro for various behavior in code.
#define LEAF_ELEMENT_TYPES \
	X(Bool) \
	X(Float) \
	X(Float2) \
	X(Float3) \
	X(Float4) \
	X(Color3) \
	X(Color4) \
	X(Matrix)

namespace GFX::Data::CBuffer
{
	enum class ElementType
	{
#define X(el) el,
		LEAF_ELEMENT_TYPES
#undef X
		Struct,
		Array,
		Empty
	};

	// Map of every leaf element type with it's properties.
	template<ElementType>
	struct Map
	{
		static constexpr bool valid = false;
	};
	// Map of every DataType used by leaf element types.
	template<typename>
	struct ReverseMap
	{
		static constexpr ElementType type = ElementType::Empty;
		static constexpr bool valid = false;
	};

#pragma region Layout Leaf Element Info
	template<> struct Map<ElementType::Bool>
	{
		using DataType = bool;
		static constexpr size_t hlslSize = 4U;
		static constexpr const char* code = "BL";
		static constexpr bool valid = true;
	};
	template<> struct Map<ElementType::Float>
	{
		using DataType = float;
		static constexpr size_t hlslSize = sizeof(DataType);
		static constexpr const char* code = "F1";
		static constexpr bool valid = true;
	};
	template<> struct Map<ElementType::Float2>
	{
		using DataType = DirectX::XMFLOAT2;
		static constexpr size_t hlslSize = sizeof(DataType);
		static constexpr const char* code = "F2";
		static constexpr bool valid = true;
	};
	template<> struct Map<ElementType::Float3>
	{
		using DataType = DirectX::XMFLOAT3;
		static constexpr size_t hlslSize = sizeof(DataType);
		static constexpr const char* code = "F3";
		static constexpr bool valid = true;
	};
	template<> struct Map<ElementType::Float4>
	{
		using DataType = DirectX::XMFLOAT4;
		static constexpr size_t hlslSize = sizeof(DataType);
		static constexpr const char* code = "F4";
		static constexpr bool valid = true;
	};
	template<> struct Map<ElementType::Color3>
	{
		using DataType = GFX::Data::ColorFloat3;
		static constexpr size_t hlslSize = sizeof(DataType);
		static constexpr const char* code = "C3";
		static constexpr bool valid = true;
	};
	template<> struct Map<ElementType::Color4>
	{
		using DataType = GFX::Data::ColorFloat4;
		static constexpr size_t hlslSize = sizeof(DataType);
		static constexpr const char* code = "C4";
		static constexpr bool valid = true;
	};
	template<> struct Map<ElementType::Matrix>
	{
		using DataType = DirectX::XMFLOAT4X4;
		static constexpr size_t hlslSize = sizeof(DataType);
		static constexpr const char* code = "M4";
		static constexpr bool valid = true;
	};

	// Sanity check to make sure that all leaf elements have corresponding Map specialization.
#define X(el) static_assert(Map<ElementType::el>::valid, "Missing Map implementation for " #el);
	LEAF_ELEMENT_TYPES
#undef X

		// Creates reverse map for every leaf type, so it is possible to find ElementType by DataType.
#define X(el) \
	template<> struct ReverseMap<Map<ElementType::el>::DataType> \
	{ \
		static constexpr ElementType type = ElementType::el; \
		static constexpr bool valid = true; \
	};
		LEAF_ELEMENT_TYPES
#undef X
#pragma endregion

	class DCBLayoutElement
	{
		friend class DCBLayout;
		friend struct ExtraData;

		struct ExtraDataBase
		{
			virtual ~ExtraDataBase() = default;
		};

		std::optional<size_t> offset;
		ElementType type = ElementType::Empty;
		std::unique_ptr<ExtraDataBase> extraData = nullptr;

		DCBLayoutElement() = default;
		DCBLayoutElement(ElementType elementType) noexcept(!IS_DEBUG);

		static constexpr size_t AdvanceToBoundary(size_t offset) noexcept;
		static constexpr bool CrossesBoundary(size_t offset, size_t size) noexcept;
		static constexpr size_t AdvanceIfCrossesBoundary(size_t offset, size_t size) noexcept;
		static inline DCBLayoutElement& GetEmptyElement() noexcept;

		// Symbol can have alphanumeric and underscore but can't start with digit
		static bool ValidateSymbol(const std::string& name) noexcept;

		size_t Finalize(size_t offsetIn) noexcept(!IS_DEBUG);

	public:
		constexpr bool Exists() const noexcept { return type != ElementType::Empty; }
		constexpr const DCBLayoutElement& ArrayType() const noexcept(!IS_DEBUG) { return const_cast<DCBLayoutElement&>(*this).ArrayType(); }
		constexpr size_t GetBeginOffset() const { return *offset; }
		inline size_t GetByteSize() const { return GetEndOffset() - GetBeginOffset(); }

		// Only for ElementType::Array
		constexpr DCBLayoutElement& ArrayType() noexcept(!IS_DEBUG);
		template<typename T>
		size_t GetLeafOffset() const noexcept(!IS_DEBUG);

		std::string GetSignature() const noexcept(!IS_DEBUG);
		size_t GetEndOffset() const noexcept(!IS_DEBUG);
		// Only for ElementType::Struct
		DCBLayoutElement& Add(ElementType typeAdded, std::string name) noexcept(!IS_DEBUG);
		// Only for ElementType::Array
		DCBLayoutElement& InitArray(ElementType addedType, size_t size) noexcept(!IS_DEBUG);
		std::pair<size_t, const DCBLayoutElement*> CalculateIndexOffset(size_t offset, size_t index) const noexcept(!IS_DEBUG);

		// Only for ElementType::Struct
		inline const DCBLayoutElement& operator[](const std::string& key) const noexcept(!IS_DEBUG) { return const_cast<DCBLayoutElement&>(*this)[key]; }

		// Only for ElementType::Struct
		DCBLayoutElement& operator[](const std::string& key) noexcept(!IS_DEBUG);
	};

	template<typename T>
	size_t DCBLayoutElement::GetLeafOffset() const noexcept(!IS_DEBUG)
	{
		switch (type)
		{
#define X(el) \
		case ElementType::el: \
		{ \
			assert(typeid(Map<ElementType::el>::DataType) == typeid(T)); \
			return *offset; \
		}
			LEAF_ELEMENT_TYPES
#undef X
		default:
		{
			assert("Tried to get offset of non-leaf element" && false);
			return 0U;
		}
		}
	}
}

typedef GFX::Data::CBuffer::ElementType DCBElementType;

#ifndef DCB_LAYOUTELEMENT_IMPL
#undef LEAF_ELEMENT_TYPES
#endif