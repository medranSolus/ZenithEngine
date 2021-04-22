#include "GFX/Data/VertexBufferData.h"

namespace GFX::Data
{
	VertexBufferData::VertexBufferData(std::shared_ptr<VertexLayout> layout, U64 size) noexcept
		: layout(layout)
	{
		assert(layout != nullptr && "VertexLayout cannot be null!");
		buffer.resize(layout->Size() * size);
	}

	VertexBufferData::VertexBufferData(std::shared_ptr<VertexLayout> layout, const aiMesh& mesh) noexcept
		: layout(layout)
	{
		assert(layout != nullptr && "VertexLayout cannot be null!");
		buffer.resize(layout->Size() * mesh.mNumVertices);
		for (U64 i = 0, size = layout->GetElementCount(); i < size; ++i)
			VertexLayout::Bridge<AttributeFill>(layout->ResolveByIndex(i).GetType(), *this, mesh, i); // TODO: Can be concurrent
	}

	void VertexBufferData::SetBoundingBox() noexcept
	{
		const U64 size = Size();
		Vector min = Math::XMLoadFloat3(&boundingBox.GetNegative());
		Vector max = Math::XMLoadFloat3(&boundingBox.GetPositive());
		for (U64 i = 0; i < size; ++i)
		{
			const auto& position = (*this)[i].Get<VertexAttribute::Position3D>();
			const Vector& pos = Math::XMLoadFloat3(&position);
			min = Math::XMVectorMin(min, pos);
			max = Math::XMVectorMax(max, pos);
		}
		Math::XMStoreFloat3(&boundingBox.GetNegative(), min);
		Math::XMStoreFloat3(&boundingBox.GetPositive(), max);
		boundingBox.Finalize();
	}

	void VertexBufferData::Resize(U64 newSize) noexcept
	{
		const U64 size = Size();
		if (size < newSize)
			buffer.resize(buffer.size() + layout->Size() * (newSize - size));
	}

	Vertex VertexBufferData::Back() noexcept
	{
		assert(buffer.size() != 0);
		return { buffer.data() + buffer.size() - layout->Size(), *layout };
	}

	Vertex VertexBufferData::Front() noexcept
	{
		assert(buffer.size() != 0);
		return { buffer.data(), *layout };
	}

	Vertex VertexBufferData::operator[](U64 i) noexcept
	{
		assert(i < Size());
		return { buffer.data() + layout->Size() * i, *layout };
	}
}