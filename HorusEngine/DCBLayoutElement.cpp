#define DCB_LAYOUTELEMENT_IMPL
#include "DCBLayoutElement.h"
#include <vector>
#include <algorithm>

namespace GFX::Data::CBuffer
{
	struct ExtraData
	{
		struct Struct : public DCBLayoutElement::ExtraDataBase
		{
			// For small number of elements it is faster than map and possibly unordered_map.
			std::vector<std::pair<std::string, DCBLayoutElement>> layoutElements;
		};
		struct Array : public DCBLayoutElement::ExtraDataBase
		{
			std::optional<DCBLayoutElement> layoutElement;
			size_t size = 0U;
		};
	};

	DCBLayoutElement::DCBLayoutElement(ElementType elementType) noexcept(!IS_DEBUG) : type(elementType)
	{
		assert(type != ElementType::Empty);
		if (type == ElementType::Struct)
			extraData = std::make_unique<ExtraData::Struct>();
		else if (type == ElementType::Array)
			extraData = std::make_unique<ExtraData::Array>();
	}

	constexpr size_t DCBLayoutElement::AdvanceToBoundary(size_t offset) noexcept
	{
		return (offset & (~static_cast<size_t>(0xFU))) + (static_cast<size_t>(static_cast<bool>(offset & 0xFU)) << 4);
	}

	constexpr bool DCBLayoutElement::CrossesBoundary(size_t offset, size_t size) noexcept
	{
		const size_t end = offset + size;
		return ((offset >> 4) != (end >> 4) && end & 0xFU) || size > 16U;
	}

	constexpr size_t DCBLayoutElement::AdvanceIfCrossesBoundary(size_t offset, size_t size) noexcept
	{
		return CrossesBoundary(offset, size) ? AdvanceToBoundary(offset) : offset;
	}

	inline DCBLayoutElement& DCBLayoutElement::GetEmptyElement() noexcept
	{
		static DCBLayoutElement empty{};
		return empty;
	}

	bool DCBLayoutElement::ValidateSymbol(const std::string& name) noexcept
	{
		return !name.empty() &&
			!std::isdigit(name.front()) &&
			std::all_of(name.begin(), name.end(), [](char c) { return std::isalnum(c) || c == '_'; });
	}

	size_t DCBLayoutElement::Finalize(size_t offsetIn) noexcept(!IS_DEBUG)
	{
		switch (type)
		{
#define X(el) \
		case ElementType::el: \
		{ \
			offset = AdvanceIfCrossesBoundary(offsetIn, Map<ElementType::el>::hlslSize); \
			return *offset + Map<ElementType::el>::hlslSize; \
		}
			LEAF_ELEMENT_TYPES
#undef X
		case ElementType::Struct:
			{
				auto& layoutElements = static_cast<ExtraData::Struct&>(*extraData).layoutElements;
				assert(layoutElements.size() != 0U);
				offset = AdvanceToBoundary(offsetIn);
				size_t offsetNext = *offset;
				for (auto& member : layoutElements)
					offsetNext = member.second.Finalize(offsetNext);
				return offsetNext;
			}
		case ElementType::Array:
		{
			auto& data = static_cast<ExtraData::Array&>(*extraData);
			assert(data.size != 0U);
			offset = AdvanceToBoundary(offsetIn);
			data.layoutElement->Finalize(*offset);
			return GetEndOffset();
		}
		default:
		{
			assert("Bad type in offset computation!" && false);
			return 0U;
		}
		}
	}

	constexpr DCBLayoutElement& DCBLayoutElement::ArrayType() noexcept(!IS_DEBUG)
	{
		assert("Accessing ArrayType of non-array" && type == ElementType::Array);
		return *static_cast<ExtraData::Array&>(*extraData).layoutElement;
	}

	std::string DCBLayoutElement::GetSignature() const noexcept(!IS_DEBUG)
	{
		switch (type)
		{
#define X(el) \
		case ElementType::el: \
			return Map<ElementType::el>::code;
			LEAF_ELEMENT_TYPES
#undef X
		case ElementType::Struct:
			{
				std::string signature = "S{";
				for (const auto& member : static_cast<ExtraData::Struct&>(*extraData).layoutElements)
					signature += member.first + ":" + member.second.GetSignature() + ";";
				return signature + "}";
			}
		case ElementType::Array:
		{
			const auto& data = static_cast<ExtraData::Array&>(*extraData);
			return "A:" + std::to_string(data.size) + "{" + data.layoutElement->GetSignature() + "}";
		}
		default:
		{
			assert("Unknow type in signature generation!" && false);
			return "?";
		}
		}
	}

	size_t DCBLayoutElement::GetEndOffset() const noexcept(!IS_DEBUG)
	{
		switch (type)
		{
#define X(el) \
		case ElementType::el: \
			return *offset + Map<ElementType::el>::hlslSize;
			LEAF_ELEMENT_TYPES
#undef X
		case ElementType::Struct:
			{
				const auto& data = static_cast<ExtraData::Struct&>(*extraData);
				return AdvanceToBoundary(data.layoutElements.back().second.GetEndOffset());
			}
		case ElementType::Array:
		{
			const auto& data = static_cast<ExtraData::Array&>(*extraData);
			return *offset + AdvanceToBoundary(data.layoutElement->GetByteSize()) * data.size;
		}
		default:
		{
			assert("Tried to get offset for empty or invalid element!" && false);
			return 0U;
		}
		}
	}

	DCBLayoutElement& DCBLayoutElement::Add(ElementType typeAdded, std::string name) noexcept(!IS_DEBUG)
	{
		assert("Cannot add inner elements into non-struct type!" && type == ElementType::Struct);
		assert("Invalid symbol name in Struct!" && ValidateSymbol(name));
		auto& layoutElements = static_cast<ExtraData::Struct&>(*extraData).layoutElements;
		for (auto& member : layoutElements)
			if (member.first == name)
				assert("Adding duplicate name to Struct!" && false);
		layoutElements.emplace_back(std::move(name), DCBLayoutElement{ typeAdded });
		return *this;
	}

	DCBLayoutElement& DCBLayoutElement::InitArray(ElementType addedType, size_t size) noexcept(!IS_DEBUG)
	{
		assert("Set on non-array in layout" && type == ElementType::Array);
		assert(size != 0U);
		auto& data = static_cast<ExtraData::Array&>(*extraData);
		data.layoutElement = { addedType };
		data.size = size;
		return *this;
	}

	std::pair<size_t, const DCBLayoutElement*> DCBLayoutElement::CalculateIndexOffset(size_t offset, size_t index) const noexcept(!IS_DEBUG)
	{
		assert("Indexing into non-array" && type == ElementType::Array);
		const auto& data = static_cast<ExtraData::Array&>(*extraData);
		assert(index < data.size);
		return { offset + 16U * index, &*data.layoutElement };
	}

	DCBLayoutElement& DCBLayoutElement::operator[](const std::string& key) noexcept(!IS_DEBUG)
	{
		assert("Cannot get inner elements in non-struct type!" && type == ElementType::Struct);
		for (auto& member : static_cast<ExtraData::Struct&>(*extraData).layoutElements)
			if (member.first == key)
				return member.second;
		return GetEmptyElement();
	}
}