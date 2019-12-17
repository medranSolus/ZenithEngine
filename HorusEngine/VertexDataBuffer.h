#pragma once
#include "Vertex.h"

namespace GFX::BasicType
{
	class VertexDataBuffer
	{
		std::vector<char> buffer;
		VertexLayout layout;

	public:
		VertexDataBuffer(VertexLayout layout, size_t size = 0U) noexcept(!IS_DEBUG);

		constexpr const VertexLayout & GetLayout() const noexcept { return layout; }
		inline const char * GetData() const noexcept(!IS_DEBUG) { return buffer.data(); }
		inline size_t Bytes() const noexcept(!IS_DEBUG) { return buffer.size(); }
		inline size_t Size() const noexcept(!IS_DEBUG) { return buffer.size() / layout.Size(); }

		void Resize(size_t newSize) noexcept(!IS_DEBUG);

		template<typename ...Params>
		void EmplaceBack(Params &&... params) noexcept(!IS_DEBUG)
		{
			assert(sizeof...(params) <= layout.GetElementCount() && "Param count doesn't match number of vertex elements!");
			buffer.resize(buffer.size() + layout.Size());
			Back().SetByIndex(0U, std::forward<Params>(params)...);
		}

		Vertex Back() noexcept(!IS_DEBUG);
		Vertex Front() noexcept(!IS_DEBUG);
		Vertex operator[](size_t i) noexcept(!IS_DEBUG);

		const Vertex Back() const noexcept(!IS_DEBUG) { const_cast<VertexDataBuffer*>(this)->Back(); }
		const Vertex Front() const noexcept(!IS_DEBUG) { const_cast<VertexDataBuffer*>(this)->Front(); }
		const Vertex operator[](size_t i) const noexcept(!IS_DEBUG) { const_cast<VertexDataBuffer&>(*this)[i]; }
	};
}
