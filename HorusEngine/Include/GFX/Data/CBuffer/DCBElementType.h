#pragma once
#include "ColorF3.h"
#include "ColorF4.h"

// List of leaf element types names.
// Each name have invocation of macro X() on it.
// Define your X() macro for various behavior in code.
#define LEAF_ELEMENT_TYPES \
	X(Bool) \
	X(SInt) \
	X(UInt) \
	X(Float) \
	X(Float2) \
	X(Float3) \
	X(Float4) \
	X(Color3) \
	X(Color4) \
	X(Matrix)

namespace GFX::Data::CBuffer
{
	enum class ElementType : U8
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
		static constexpr bool VALID = false;
	};

	// Map of every DataType used by leaf element types.
	template<typename>
	struct ReverseMap
	{
		static constexpr ElementType TYPE = ElementType::Empty;
		static constexpr bool VALID = false;
	};

#pragma region Layout Leaf Element Info
	template<> struct Map<ElementType::Bool>
	{
		using DataType = bool;
		static constexpr U64 GPU_SIZE = 4;
		static constexpr const char* CODE = "B";
		static constexpr bool VALID = true;
	};
	template<> struct Map<ElementType::SInt>
	{
		using DataType = S32;
		static constexpr U64 GPU_SIZE = sizeof(DataType);
		static constexpr const char* CODE = "S";
		static constexpr bool VALID = true;
	};
	template<> struct Map<ElementType::UInt>
	{
		using DataType = U32;
		static constexpr U64 GPU_SIZE = sizeof(DataType);
		static constexpr const char* CODE = "U";
		static constexpr bool VALID = true;
	};
	template<> struct Map<ElementType::Float>
	{
		using DataType = float;
		static constexpr U64 GPU_SIZE = sizeof(DataType);
		static constexpr const char* CODE = "F";
		static constexpr bool VALID = true;
	};
	template<> struct Map<ElementType::Float2>
	{
		using DataType = Float2;
		static constexpr U64 GPU_SIZE = sizeof(DataType);
		static constexpr const char* CODE = "F2";
		static constexpr bool VALID = true;
	};
	template<> struct Map<ElementType::Float3>
	{
		using DataType = Float3;
		static constexpr U64 GPU_SIZE = sizeof(DataType);
		static constexpr const char* CODE = "F3";
		static constexpr bool VALID = true;
	};
	template<> struct Map<ElementType::Float4>
	{
		using DataType = Float4;
		static constexpr U64 GPU_SIZE = sizeof(DataType);
		static constexpr const char* CODE = "F4";
		static constexpr bool VALID = true;
	};
	template<> struct Map<ElementType::Color3>
	{
		using DataType = ColorF3;
		static constexpr U64 GPU_SIZE = sizeof(DataType);
		static constexpr const char* CODE = "C3";
		static constexpr bool VALID = true;
	};
	template<> struct Map<ElementType::Color4>
	{
		using DataType = ColorF4;
		static constexpr U64 GPU_SIZE = sizeof(DataType);
		static constexpr const char* CODE = "C";
		static constexpr bool VALID = true;
	};
	template<> struct Map<ElementType::Matrix>
	{
		using DataType = Float4x4;
		static constexpr U64 GPU_SIZE = sizeof(DataType);
		static constexpr const char* CODE = "M";
		static constexpr bool VALID = true;
	};

	// Sanity check to make sure that all leaf elements have corresponding Map specialization.
#define X(el) static_assert(Map<ElementType::el>::VALID, "Missing Map implementation for " #el);
	LEAF_ELEMENT_TYPES
#undef X

	// Create reverse map for every leaf type, so it is possible to find ElementType by DataType.
#define X(el) \
	template<> struct ReverseMap<Map<ElementType::el>::DataType> \
	{ \
		static constexpr ElementType TYPE = ElementType::el; \
		static constexpr bool VALID = true; \
	};
		LEAF_ELEMENT_TYPES
#undef X
#pragma endregion
}

typedef GFX::Data::CBuffer::ElementType DCBElementType;