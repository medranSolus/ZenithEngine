#include "VertexBufferData.h"

namespace GFX::Data
{
	VertexBufferData::VertexBufferData(std::shared_ptr<VertexLayout> layout, size_t size) noexcept(!IS_DEBUG) : layout(layout)
	{
		assert(layout != nullptr && "VertexLayout cannot be null!");
		buffer.resize(layout->Size() * size);
	}

	void VertexBufferData::Resize(size_t newSize) noexcept(!IS_DEBUG)
	{
		const size_t size = Size();
		if (size < newSize)
			buffer.resize(buffer.size() + layout->Size() * (newSize - size));
	}

	Vertex VertexBufferData::Back() noexcept(!IS_DEBUG)
	{
		assert(buffer.size() != 0U);
		return Vertex{ buffer.data() + buffer.size() - layout->Size(), *layout };
	}

	Vertex VertexBufferData::Front() noexcept(!IS_DEBUG)
	{
		assert(buffer.size() != 0U);
		return Vertex{ buffer.data(), *layout };
	}

	Vertex VertexBufferData::operator[](size_t i) noexcept(!IS_DEBUG)
	{
		assert(i < Size());
		return Vertex{ buffer.data() + layout->Size() * i, *layout };
	}
}