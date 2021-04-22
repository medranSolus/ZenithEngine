#define _DCB_LAYOUT_ELEMENT_IMPL
#include "GFX/Data/CBuffer/DCBLayoutElement.h"

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
			DCBLayoutElement layoutElement;
			U64 size = 0;
		};
	};

	constexpr U64 DCBLayoutElement::AdvanceToBoundary(U64 offset) noexcept
	{
		return (offset & (~static_cast<U64>(0xF))) + (static_cast<U64>(static_cast<bool>(offset & 0xF)) << 4);
	}

	constexpr bool DCBLayoutElement::CrossesBoundary(U64 offset, U64 size) noexcept
	{
		const U64 end = offset + size;
		return ((offset >> 4) != (end >> 4) && end & 0xF) || size > 16;
	}

	constexpr U64 DCBLayoutElement::AdvanceIfCrossesBoundary(U64 offset, U64 size) noexcept
	{
		return CrossesBoundary(offset, size) ? AdvanceToBoundary(offset) : offset;
	}

	bool DCBLayoutElement::ValidateSymbol(const std::string& name) noexcept
	{
		return !name.empty() && !std::isdigit(name.front()) &&
			std::all_of(name.begin(), name.end(), [](char c) { return std::isalnum(c) || c == '_'; });
	}

	U64 DCBLayoutElement::Finalize(U64 offsetIn) noexcept
	{
		switch (type)
		{
#define X(el) \
		case ElementType::el: \
		{ \
			offset = AdvanceIfCrossesBoundary(offsetIn, Map<ElementType::el>::GPU_SIZE); \
			return offset + Map<ElementType::el>::GPU_SIZE; \
		}
		LEAF_ELEMENT_TYPES
#undef X
		case ElementType::Struct:
		{
			auto& layoutElements = static_cast<ExtraData::Struct&>(*extraData).layoutElements;
			assert(layoutElements.size() != 0);
			offset = AdvanceToBoundary(offsetIn);
			U64 offsetNext = offset;
			for (auto& member : layoutElements)
				offsetNext = member.second.Finalize(offsetNext);
			return offsetNext;
		}
		case ElementType::Array:
		{
			auto& data = static_cast<ExtraData::Array&>(*extraData);
			assert(data.size != 0);
			offset = AdvanceToBoundary(offsetIn);
			data.layoutElement.Finalize(offset);
			return GetEndOffset();
		}
		default:
		{
			assert("Bad type in offset computation!" && false);
			return 0;
		}
		}
	}

	DCBLayoutElement::DCBLayoutElement(ElementType elementType) noexcept : type(elementType)
	{
		assert(type != ElementType::Empty);
		if (type == ElementType::Struct)
			extraData = std::make_unique<ExtraData::Struct>();
		else if (type == ElementType::Array)
			extraData = std::make_unique<ExtraData::Array>();
	}

	std::string DCBLayoutElement::GetSignature() const noexcept
	{
		switch (type)
		{
#define X(el) \
		case ElementType::el: \
			return Map<ElementType::el>::CODE;
			LEAF_ELEMENT_TYPES
#undef X
		case ElementType::Struct:
		{
			std::string signature = "S{";
			for (const auto& member : static_cast<ExtraData::Struct&>(*extraData).layoutElements)
				signature += member.first + ":" + member.second.GetSignature() + ";";
			signature.back() = '}';
			return signature;
		}
		case ElementType::Array:
		{
			const auto& data = static_cast<ExtraData::Array&>(*extraData);
			return "A" + std::to_string(data.size) + "{" + data.layoutElement.GetSignature() + "}";
		}
		default:
		{
			assert("Unknow type in signature generation!" && false);
			return "?";
		}
		}
	}

	U64 DCBLayoutElement::GetEndOffset() const noexcept
	{
		switch (type)
		{
#define X(el) \
		case ElementType::el: \
			return offset + Map<ElementType::el>::GPU_SIZE;
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
			const U64 byteSize = data.layoutElement.GetByteSize();
			return offset + AdvanceToBoundary(byteSize) * (data.size - 1) + byteSize;
		}
		default:
		{
			assert("Tried to get offset for empty or invalid element!" && false);
			return 0;
		}
		}
	}

	std::pair<U64, const DCBLayoutElement*> DCBLayoutElement::CalculateIndexOffset(U64 offset, U64 index) const noexcept
	{
		assert("Indexing into non-array" && type == ElementType::Array);
		const auto& data = static_cast<ExtraData::Array&>(*extraData);
		assert(index < data.size);
		return { offset + (data.layoutElement.type == ElementType::Matrix ? 64 : 16) * index, &data.layoutElement };
	}

	DCBLayoutElement& DCBLayoutElement::ArrayType() noexcept
	{
		assert("Accessing ArrayType of non-array" && type == ElementType::Array);
		return reinterpret_cast<ExtraData::Array&>(*extraData).layoutElement;
	}

	DCBLayoutElement& DCBLayoutElement::InitArray(ElementType addedType, U64 size) noexcept
	{
		assert("Set on non-array in layout" && type == ElementType::Array);
		assert(size != 0);
		auto& data = static_cast<ExtraData::Array&>(*extraData);
		data.layoutElement = DCBLayoutElement(addedType);
		data.size = size;
		return *this;
	}

	DCBLayoutElement& DCBLayoutElement::Add(ElementType typeAdded, std::string name) noexcept
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

	DCBLayoutElement& DCBLayoutElement::operator[](const std::string& key) noexcept
	{
		assert("Cannot get inner elements in non-struct type!" && type == ElementType::Struct);
		for (auto& member : static_cast<ExtraData::Struct&>(*extraData).layoutElements)
			if (member.first == key)
				return member.second;
		return GetEmptyElement();
	}
}