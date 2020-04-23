#pragma once
#include "DCBElementConst.h"

namespace GFX::Data::CBuffer
{
	class DCBElement
	{
		friend class DynamicCBuffer;

	public:
		class Ptr
		{
			friend class DCBElement;

			DCBElement* element = nullptr;

			inline Ptr(DCBElement* element) noexcept : element(element) {}

		public:
			~Ptr() = default;

			template<typename T>
			operator T* () const noexcept;
		};

	private:
		const DCBLayoutElement* layout = nullptr;
		char* bytes = nullptr;
		size_t offset = 0U;

		inline DCBElement(const DCBLayoutElement* layout, char* bytes, size_t offset) noexcept
			: layout(layout), bytes(bytes), offset(offset) {}

	public:
		~DCBElement() = default;

		constexpr bool Exists() const noexcept { return layout->Exists(); }

		template<typename T>
		constexpr bool SetIfExists(const T& val) noexcept;

		inline operator DCBElementConst() const noexcept { return { layout, bytes, offset }; }
		inline DCBElement operator[](const std::string& key) const noexcept(!IS_DEBUG) { return { &(*layout)[key], bytes, offset }; }
		inline Ptr operator&() const noexcept { return Ptr{ const_cast<DCBElement*>(this) }; }

		template<typename T>
		operator T& () noexcept(!IS_DEBUG);
		template<typename T>
		T& operator=(const T& rhs) noexcept;
		DCBElement operator[](size_t index) const noexcept(!IS_DEBUG);
	};

	template<typename T>
	DCBElement::Ptr::operator T* () const noexcept
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
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
	DCBElement::operator T& () noexcept(!IS_DEBUG)
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported DataType used in conversion!");
		return *reinterpret_cast<T*>(bytes + offset + layout->GetLeafOffset<T>());
	}

	template<typename T>
	T& DCBElement::operator=(const T& val) noexcept
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported DataType used in assignment!");
		return static_cast<T&>(*this) = val;
	}
}