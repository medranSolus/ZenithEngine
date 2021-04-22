#pragma once
#include "VertexLayout.h"
#include <type_traits>

namespace GFX::Data
{
	class Vertex final
	{
		friend class VertexBufferData;

		template<VertexAttribute Type>
		struct AttributeSetting
		{
			template<typename T>
			static constexpr void Exec(Vertex* vertex, U8* attribute, T&& val) noexcept { vertex->Set<Type>(attribute, std::forward<T>(val)); }
		};

		U8* data = nullptr;
		const VertexLayout& layout;

		// Check for proper parameter
		template<VertexLayout::ElementType T, typename SrcType>
		constexpr void Set(U8* attribute, SrcType&& val) noexcept;

		constexpr Vertex(U8* data, const VertexLayout& layout) noexcept : data(data), layout(layout) { assert(data != nullptr); }

	public:
		Vertex(Vertex&&) = default;
		Vertex(const Vertex&) = default;
		Vertex& operator=(Vertex&&) = default;
		Vertex& operator=(const Vertex&) = default;
		~Vertex() = default;

		template<VertexLayout::ElementType T>
		constexpr const auto& Get() const noexcept { return const_cast<Vertex&>(*this).Get(); }

		template<VertexLayout::ElementType T>
		constexpr auto& Get() noexcept;

		template<typename T>
		constexpr void SetByIndex(U64 i, T&& val) noexcept;
		// Enables parameter pack setting of multiple parameters by element index
		template<typename First, typename ...Rest>
		constexpr void SetByIndex(U64 i, First&& first, Rest&&... rest) noexcept;
	};

#pragma region Functions
	template<VertexLayout::ElementType T, typename SrcType>
	constexpr void Vertex::Set(U8* attribute, SrcType&& val) noexcept
	{
		using Dest = typename VertexLayout::Desc<T>::DataType;
		if constexpr (std::is_assignable<Dest, SrcType>::value)
			*reinterpret_cast<Dest*>(attribute) = val;
		else
			assert("Parameter attribute type mismatch" && false);
	}

	template<VertexLayout::ElementType T>
	constexpr auto& Vertex::Get() noexcept
	{
		return *reinterpret_cast<typename VertexLayout::Desc<T>::DataType*>(data + layout.Resolve(T).GetOffset());
	}

	template<typename T>
	constexpr void Vertex::SetByIndex(U64 i, T&& val) noexcept
	{
		const auto& element = layout.ResolveByIndex(i);
		VertexLayout::Bridge<AttributeSetting>(element.GetType(), this, data + element.GetOffset(), std::forward<T>(val));
	}

	template<typename First, typename ...Rest>
	constexpr void Vertex::SetByIndex(U64 i, First&& first, Rest&& ...rest) noexcept
	{
		SetByIndex(i, std::forward<First>(first));
		SetByIndex(i + 1, std::forward<Rest>(rest)...);
	}
#pragma endregion
}