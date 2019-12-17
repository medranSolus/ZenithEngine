#include "VertexDataBuffer.h"

namespace GFX::BasicType
{
	VertexDataBuffer::VertexDataBuffer(VertexLayout layout, size_t size) noexcept(!IS_DEBUG) : layout(std::move(layout))
	{
		buffer.resize(this->layout.Size() * (size));
	}

	void VertexDataBuffer::Resize(size_t newSize) noexcept(!IS_DEBUG)
	{
		const size_t size = Size();
		if (size < newSize)
			buffer.resize(buffer.size() + layout.Size() * (newSize - size));
	}

	Vertex VertexDataBuffer::Back() noexcept(!IS_DEBUG)
	{
		assert(buffer.size() != 0U);
		return Vertex{ buffer.data() + buffer.size() - layout.Size(), layout };
	}

	Vertex VertexDataBuffer::Front() noexcept(!IS_DEBUG)
	{
		assert(buffer.size() != 0U);
		return Vertex{ buffer.data(), layout };
	}

	Vertex VertexDataBuffer::operator[](size_t i) noexcept(!IS_DEBUG)
	{
		assert(i < Size());
		return Vertex{ buffer.data() + layout.Size() * i, layout };
	}
}
