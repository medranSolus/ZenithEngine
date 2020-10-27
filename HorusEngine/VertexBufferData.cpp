#include "VertexBufferData.h"

namespace GFX::Data
{
	VertexBufferData::VertexBufferData(std::shared_ptr<VertexLayout> layout, size_t size) noexcept(!IS_DEBUG)
		: layout(layout)
	{
		assert(layout != nullptr && "VertexLayout cannot be null!");
		buffer.resize(layout->Size() * size);
	}

	VertexBufferData::VertexBufferData(std::shared_ptr<VertexLayout> layout, const aiMesh& mesh) noexcept(!IS_DEBUG)
		: layout(layout)
	{
		assert(layout != nullptr && "VertexLayout cannot be null!");
		buffer.resize(layout->Size() * mesh.mNumVertices);
		for (size_t i = 0, size = layout->GetElementCount(); i < size; ++i)
			VertexLayout::Bridge<AttributeFill>(layout->ResolveByIndex(i).GetType(), *this, mesh, i); // TODO: Can be concurrent
	}

	void VertexBufferData::SetBoundingBox() noexcept(!IS_DEBUG)
	{
		const size_t size = Size();
		DirectX::XMVECTOR min = DirectX::XMLoadFloat3(&boundingBox.GetNegative());
		DirectX::XMVECTOR max = DirectX::XMLoadFloat3(&boundingBox.GetPositive());
		for (size_t i = 0; i < size; ++i)
		{
			const auto& position = (*this)[i].Get<VertexAttribute::Position3D>();
			const DirectX::XMVECTOR& pos = DirectX::XMLoadFloat3(&position);
			min = DirectX::XMVectorMin(min, pos);
			max = DirectX::XMVectorMax(max, pos);
		}
		DirectX::XMStoreFloat3(&boundingBox.GetNegative(), min);
		DirectX::XMStoreFloat3(&boundingBox.GetPositive(), max);
		boundingBox.Finalize();
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