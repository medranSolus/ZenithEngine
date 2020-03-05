#pragma once
#include "DCBLayoutElement.h"

namespace GFX::Data::CBuffer
{
	class DCBElementConst
	{
		friend class DynamicCBuffer;
		friend class DCBElement;

	public:
		class Ptr
		{
			friend class DCBElementConst;

			const DCBElementConst* element = nullptr;

			inline Ptr(const DCBElementConst* element) noexcept : element(element) {}

		public:
			template<typename T>
			operator const T* () const noexcept(!IS_DEBUG);
		private:
		};

	private:
		const DCBLayoutElement* layout = nullptr;
		char* bytes = nullptr;
		size_t offset = 0U;

		inline DCBElementConst(const DCBLayoutElement* layout, char* bytes, size_t offset) noexcept
			: layout(layout), bytes(bytes), offset(offset) {}

	public:
		constexpr bool Exists() const noexcept { return layout->Exists(); }

		inline DCBElementConst operator[](const std::string& key) const noexcept(!IS_DEBUG) { return { &(*layout)[key], bytes, offset }; }
		inline Ptr operator&() const noexcept(!IS_DEBUG) { return Ptr{ this }; }

		template<typename T>
		operator const T& () const noexcept(!IS_DEBUG);
		DCBElementConst operator[](size_t index) const noexcept(!IS_DEBUG);
	};

	template<typename T>
	DCBElementConst::Ptr::operator const T* () const noexcept(!IS_DEBUG)
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported DataType used in pointer conversion!");
		return &static_cast<const T&>(*element);
	}

	template<typename T>
	DCBElementConst::operator const T& () const noexcept(!IS_DEBUG)
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported DataType used in conversion!");
		return *reinterpret_cast<T*>(bytes + offset + layout->GetLeafOffset<T>());
	}
}