#pragma once
#include "DCBElementType.h"
#include <string>
#include <memory>

namespace GFX::Data::CBuffer
{
	class DCBLayoutElement final
	{
		friend class DCBLayout;
		friend struct ExtraData;

		struct ExtraDataBase { virtual ~ExtraDataBase() = default; };

		U64 offset = 0;
		ElementType type = ElementType::Empty;
		std::unique_ptr<ExtraDataBase> extraData = nullptr;

		static DCBLayoutElement& GetEmptyElement() noexcept { static DCBLayoutElement empty; return empty; }

		static constexpr U64 AdvanceToBoundary(U64 offset) noexcept;
		static constexpr bool CrossesBoundary(U64 offset, U64 size) noexcept;
		static constexpr U64 AdvanceIfCrossesBoundary(U64 offset, U64 size) noexcept;

		// Symbol can have alphanumeric and underscore but can't start with digit
		static bool ValidateSymbol(const std::string& name) noexcept;

		U64 Finalize(U64 offsetIn) noexcept;

		DCBLayoutElement() = default;
		DCBLayoutElement(ElementType elementType) noexcept;

	public:
		DCBLayoutElement(DCBLayoutElement&&) = default;
		DCBLayoutElement(const DCBLayoutElement&) = default;
		DCBLayoutElement& operator=(DCBLayoutElement&&) = default;
		DCBLayoutElement& operator=(const DCBLayoutElement&) = default;
		~DCBLayoutElement() = default;

		constexpr bool Exists() const noexcept { return type != ElementType::Empty; }
		constexpr U64 GetBeginOffset() const { return offset; }
		// Only for DCBElementType::Array
		const DCBLayoutElement& ArrayType() const noexcept { return const_cast<DCBLayoutElement&>(*this).ArrayType(); }
		U64 GetByteSize() const { return GetEndOffset() - GetBeginOffset(); }

		template<typename T>
		constexpr U64 GetLeafOffset() const noexcept;

		std::string GetSignature() const noexcept;
		U64 GetEndOffset() const noexcept;
		std::pair<U64, const DCBLayoutElement*> CalculateIndexOffset(U64 offset, U64 index) const noexcept;

		// Only for DCBElementType::Array
		DCBLayoutElement& ArrayType() noexcept;
		// Only for DCBElementType::Array
		DCBLayoutElement& InitArray(ElementType addedType, U64 size) noexcept;
		// Only for DCBElementType::Struct
		DCBLayoutElement& Add(ElementType typeAdded, std::string name) noexcept;

		// Only for DCBElementType::Struct
		const DCBLayoutElement& operator[](const std::string& key) const noexcept { return const_cast<DCBLayoutElement&>(*this)[key]; }
		// Only for DCBElementType::Struct
		DCBLayoutElement& operator[](const std::string& key) noexcept;
	};

#pragma region Functions
	template<typename T>
	constexpr U64 DCBLayoutElement::GetLeafOffset() const noexcept
	{
		switch (type)
		{
#define X(el) \
		case ElementType::el: \
		{ \
			assert(typeid(Map<ElementType::el>::DataType) == typeid(T)); \
			return offset; \
		}
		LEAF_ELEMENT_TYPES
#undef X
		default:
		{
			assert("Tried to get offset of non-leaf element" && false);
			return 0;
		}
		}
	}
#pragma endregion
}

#ifndef _DCB_LAYOUT_ELEMENT_IMPL
#undef LEAF_ELEMENT_TYPES
#endif