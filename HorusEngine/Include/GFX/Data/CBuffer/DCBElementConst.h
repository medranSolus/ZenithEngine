#pragma once
#include "DCBLayoutElement.h"

namespace GFX::Data::CBuffer
{
	class DCBElementConst final
	{
		friend class DynamicCBuffer;
		friend class DCBElement;

	public:
		class Ptr final
		{
			friend class DCBElementConst;

			const DCBElementConst* element = nullptr;

			constexpr Ptr(const DCBElementConst* element) noexcept : element(element) {}
			Ptr(const Ptr&) = delete;
			Ptr& operator=(const Ptr&) = delete;

		public:
			~Ptr() = default;

			template<typename T>
			constexpr operator const T* () const noexcept;
		};

	private:
		const DCBLayoutElement* layout = nullptr;
		U8* bytes = nullptr;
		U64 offset = 0;

		constexpr DCBElementConst(const DCBLayoutElement* layout, U8* bytes, U64 offset) noexcept
			: layout(layout), bytes(bytes), offset(offset) {}

	public:
		DCBElementConst(DCBElementConst&&) = default;
		DCBElementConst(const DCBElementConst&) = default;
		DCBElementConst& operator=(DCBElementConst&&) = default;
		DCBElementConst& operator=(const DCBElementConst&) = default;
		~DCBElementConst() = default;

		constexpr bool Exists() const noexcept { return layout->Exists(); }

		constexpr Ptr operator&() const noexcept { return Ptr{ this }; }
		DCBElementConst operator[](const std::string& key) const noexcept { return { &(*layout)[key], bytes, offset }; }

		template<typename T>
		constexpr operator const T& () const noexcept;

		DCBElementConst operator[](U64 index) const noexcept;
	};

#pragma region Functions
	template<typename T>
	constexpr DCBElementConst::Ptr::operator const T* () const noexcept
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::VALID, "Unsupported DataType used in pointer conversion!");
		return &static_cast<const T&>(*element);
	}

	template<typename T>
	constexpr DCBElementConst::operator const T& () const noexcept
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::VALID, "Unsupported DataType used in conversion!");
		return *reinterpret_cast<T*>(bytes + offset + layout->GetLeafOffset<T>());
	}
#pragma endregion
}