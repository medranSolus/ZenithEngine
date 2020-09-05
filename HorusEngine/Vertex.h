#pragma once
#include "VertexLayout.h"
#include <type_traits>

namespace GFX::Data
{
	class Vertex
	{
		friend class VertexBufferData;

		template<VertexAttribute Type>
		struct AttributeSetting
		{
			template<typename T>
			static constexpr void Exec(Vertex* vertex, char* attribute, T&& val) noexcept(!IS_DEBUG) { vertex->Set<Type>(attribute, std::forward<T>(val)); }
		};

		char* data = nullptr;
		const VertexLayout& layout;

		// Enables parameter pack setting of multiple parameters by element index
		template<typename First, typename ...Rest>
		constexpr void SetByIndex(size_t i, First&& first, Rest&&... rest) noexcept(!IS_DEBUG);

		// Check for proper parameter
		template<VertexLayout::ElementType T, typename SrcType>
		constexpr void Set(char* attribute, SrcType&& val) noexcept(!IS_DEBUG);

	protected:
		Vertex(char* data, const VertexLayout& layout) noexcept(!IS_DEBUG);

	public:
		Vertex(const Vertex&) = default;
		Vertex& operator=(const Vertex&) = default;
		~Vertex() = default;

		template<VertexLayout::ElementType T>
		inline auto& Get() noexcept(!IS_DEBUG)
		{
			return *reinterpret_cast<typename VertexLayout::Desc<T>::DataType*>(data + layout.Resolve(T).GetOffset());
		}

		template<VertexLayout::ElementType T>
		inline const auto& Get() const noexcept(!IS_DEBUG)
		{
			return *const_cast<typename VertexLayout::Desc<T>::DataType*>(data + layout.Resolve(T).GetOffset());
		}

		template<typename T>
		constexpr void SetByIndex(size_t i, T&& val) noexcept(!IS_DEBUG);
	};

	template<typename First, typename ...Rest>
	constexpr void Vertex::SetByIndex(size_t i, First&& first, Rest&& ...rest) noexcept(!IS_DEBUG)
	{
		SetByIndex(i, std::forward<First>(first));
		SetByIndex(i + 1, std::forward<Rest>(rest)...);
	}

	template<VertexLayout::ElementType T, typename SrcType>
	constexpr void Vertex::Set(char* attribute, SrcType&& val) noexcept(!IS_DEBUG)
	{
		using Dest = typename VertexLayout::Desc<T>::DataType;
		if constexpr (std::is_assignable<Dest, SrcType>::value)
			*reinterpret_cast<Dest*>(attribute) = val;
		else
			assert("Parameter attribute type mismatch" && false);
	}

	template<typename T>
	constexpr void Vertex::SetByIndex(size_t i, T&& val) noexcept(!IS_DEBUG)
	{
		const auto& element = layout.ResolveByIndex(i);
		VertexLayout::Bridge<AttributeSetting>(element.GetType(), this, data + element.GetOffset(), std::forward<T>(val));
	}
}