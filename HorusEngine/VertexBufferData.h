#pragma once
#include "Vertex.h"
#include <memory>

namespace GFX::Data
{
	class VertexBufferData
	{
		std::vector<char> buffer;
		std::shared_ptr<VertexLayout> layout = nullptr;

	public:
		VertexBufferData() noexcept {}
		VertexBufferData(std::shared_ptr<VertexLayout> layout, size_t size = 0U) noexcept(!IS_DEBUG);
		VertexBufferData(const VertexBufferData&) = default;
		VertexBufferData& operator=(const VertexBufferData&) = default;
		~VertexBufferData() = default;

		inline std::shared_ptr<VertexLayout> GetLayout() const noexcept { return layout; }
		inline const char* GetData() const noexcept { return buffer.data(); }
		inline size_t Bytes() const noexcept { return buffer.size(); }
		inline size_t Size() const noexcept(!IS_DEBUG) { return buffer.size() / layout->Size(); }
		inline void Reserve(size_t capacity) noexcept(!IS_DEBUG) { buffer.reserve(capacity * layout->Size()); }

		void Resize(size_t newSize) noexcept(!IS_DEBUG);

		template<typename ...Params>
		void EmplaceBack(Params&&... params) noexcept(!IS_DEBUG);

		Vertex Back() noexcept(!IS_DEBUG);
		Vertex Front() noexcept(!IS_DEBUG);
		Vertex operator[](size_t i) noexcept(!IS_DEBUG);

		const Vertex Back() const noexcept(!IS_DEBUG) { const_cast<VertexBufferData*>(this)->Back(); }
		const Vertex Front() const noexcept(!IS_DEBUG) { const_cast<VertexBufferData*>(this)->Front(); }
		const Vertex operator[](size_t i) const noexcept(!IS_DEBUG) { const_cast<VertexBufferData&>(*this)[i]; }
	};

	template<typename ...Params>
	void VertexBufferData::EmplaceBack(Params&& ...params) noexcept(!IS_DEBUG)
	{
		assert(sizeof...(params) <= layout->GetElementCount() && "Param count doesn't match number of vertex elements!");
		buffer.resize(buffer.size() + layout->Size());
		Back().SetByIndex(0U, std::forward<Params>(params)...);
	}
}