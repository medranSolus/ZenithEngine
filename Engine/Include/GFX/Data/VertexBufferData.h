#pragma once
#include "Vertex.h"
#include "BoundingBox.h"
#include <memory>

namespace ZE::GFX::Data
{
	class VertexBufferData final
	{
		template<VertexAttribute Type>
		struct AttributeFill
		{
			static constexpr void Exec(VertexBufferData& buffer, const aiMesh& mesh, U64 index) noexcept;
		};

		std::vector<U8> buffer;
		BoundingBox boundingBox;
		std::shared_ptr<VertexLayout> layout = nullptr;

	public:
		VertexBufferData() = default;
		VertexBufferData(std::shared_ptr<VertexLayout> layout, U64 size = 0) noexcept;
		VertexBufferData(std::shared_ptr<VertexLayout> layout, const aiMesh& mesh) noexcept;
		VertexBufferData(VertexBufferData&&) = default;
		VertexBufferData(const VertexBufferData&) = default;
		VertexBufferData& operator=(VertexBufferData&&) = default;
		VertexBufferData& operator=(const VertexBufferData&) = default;
		~VertexBufferData() = default;

		constexpr const BoundingBox& GetBox() const noexcept { return boundingBox; }
		constexpr void SetBox(const BoundingBox& box) noexcept { boundingBox = box; }

		const U8* GetData() const noexcept { return buffer.data(); }
		std::shared_ptr<VertexLayout> GetLayout() const noexcept { return layout; }

		U64 Bytes() const noexcept { return buffer.size(); }
		U64 Size() const noexcept { return buffer.size() / layout->Size(); }
		void Reserve(U64 capacity) noexcept { buffer.reserve(capacity * layout->Size()); }

		const Vertex Back() const noexcept { const_cast<VertexBufferData*>(this)->Back(); }
		const Vertex Front() const noexcept { const_cast<VertexBufferData*>(this)->Front(); }
		const Vertex operator[](U64 i) const noexcept { const_cast<VertexBufferData&>(*this)[i]; }

		template<typename ...Params>
		void EmplaceBack(Params&&... params) noexcept;

		void SetBoundingBox() noexcept;
		void Resize(U64 newSize) noexcept;
		Vertex Back() noexcept;
		Vertex Front() noexcept;
		Vertex operator[](U64 i) noexcept;
	};

#pragma region Function
	template<VertexAttribute Type>
	constexpr void VertexBufferData::AttributeFill<Type>::Exec(VertexBufferData& buffer, const aiMesh& mesh, U64 index) noexcept
	{
		for (U32 size = mesh.mNumVertices, i = 0; i < size; ++i)
			buffer[i].SetByIndex(index, VertexLayout::Desc<Type>::Extract(mesh, i));
	}

	template<>
	struct VertexBufferData::AttributeFill<VertexAttribute::Position3D>
	{
		static void Exec(VertexBufferData& buffer, const aiMesh& mesh, U64 index) noexcept
		{
			Vector min = Math::XMLoadFloat3(&buffer.boundingBox.GetNegative());
			Vector max = Math::XMLoadFloat3(&buffer.boundingBox.GetPositive());
			for (U32 size = mesh.mNumVertices, i = 0; i < size; ++i)
			{
				const auto& position = VertexLayout::Desc<VertexAttribute::Position3D>::Extract(mesh, i);
				const Vector& pos = Math::XMLoadFloat3(&position);
				min = Math::XMVectorMin(min, pos);
				max = Math::XMVectorMax(max, pos);
				buffer[i].SetByIndex(index, position);
			}
			Math::XMStoreFloat3(&buffer.boundingBox.GetNegative(), min);
			Math::XMStoreFloat3(&buffer.boundingBox.GetPositive(), max);
			buffer.boundingBox.Finalize();
		}
	};

	template<typename ...Params>
	void VertexBufferData::EmplaceBack(Params&& ...params) noexcept
	{
		assert(sizeof...(params) <= layout->GetElementCount() && "Param count doesn't match number of vertex elements!");
		buffer.resize(buffer.size() + layout->Size());
		Back().SetByIndex(0, std::forward<Params>(params)...);
	}
#pragma endregion
}