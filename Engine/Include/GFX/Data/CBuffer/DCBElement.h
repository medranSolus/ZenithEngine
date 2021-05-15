#pragma once
#include "DCBElementConst.h"

namespace ZE::GFX::Data::CBuffer
{
	class DCBElement final
	{
		friend class DynamicCBuffer;

	public:
		class Ptr final
		{
			friend class DCBElement;

			DCBElement* element = nullptr;

			constexpr Ptr(DCBElement* element) noexcept : element(element) {}
			Ptr(const Ptr&) = delete;
			Ptr& operator=(const Ptr&) = delete;

		public:
			~Ptr() = default;

			template<typename T>
			constexpr operator T* () const noexcept;
		};

	private:
		const DCBLayoutElement* layout = nullptr;
		U8* bytes = nullptr;
		U64 offset = 0;

		constexpr DCBElement(const DCBLayoutElement* layout, U8* bytes, U64 offset) noexcept
			: layout(layout), bytes(bytes), offset(offset) {}

	public:
		DCBElement(DCBElement&&) = default;
		DCBElement(const DCBElement&) = default;
		DCBElement& operator=(DCBElement&&) = default;
		DCBElement& operator=(const DCBElement&) = default;
		~DCBElement() = default;

		constexpr bool Exists() const noexcept { return layout->Exists(); }
		constexpr Ptr operator&() const noexcept { return Ptr{ const_cast<DCBElement*>(this) }; }
		constexpr operator DCBElementConst() const noexcept { return { layout, bytes, offset }; }

		DCBElement operator[](const std::string& key) const noexcept { return { &(*layout)[key], bytes, offset }; }

		template<typename T>
		constexpr bool SetIfExists(const T& val) noexcept;
		template<typename T>
		constexpr operator T& () noexcept;
		template<typename T>
		constexpr T& operator=(const T& val) noexcept;

		DCBElement operator[](U64 index) const noexcept;
	};

#pragma region Functions
	template<typename T>
	constexpr DCBElement::Ptr::operator T* () const noexcept
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::VALID, "Unsupported SysType used in pointer conversion");
		return &static_cast<T&>(*element);
	}

	template<typename T>
	constexpr bool DCBElement::SetIfExists(const T& val) noexcept
	{
		if (Exists())
		{
			*this = val;
			return true;
		}
		return false;
	}

	template<typename T>
	constexpr DCBElement::operator T& () noexcept
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::VALID, "Unsupported DataType used in conversion!");
		return *reinterpret_cast<T*>(bytes + offset + layout->GetLeafOffset<T>());
	}

	template<typename T>
	constexpr T& DCBElement::operator=(const T& val) noexcept
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::VALID, "Unsupported DataType used in assignment!");
		return static_cast<T&>(*this) = val;
	}
#pragma endregion
}